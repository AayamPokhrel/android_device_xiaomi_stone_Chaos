/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.qti.media.c2@1.0-service"

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <minijail.h>
#include <dlfcn.h>
#include <signal.h>
#include <unistd.h>

#include <C2Component.h>
#include <codec2/hidl/1.0/ComponentStore.h>

static constexpr char kBaseSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/codec2.vendor.base-arm64.policy";
static constexpr char kExtSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/codec2.vendor.ext-arm64.policy";

// QC2ComponentStore::Get() returns std::shared_ptr<C2ComponentStore>
typedef std::shared_ptr<C2ComponentStore> (*QC2GetStoreFunc)();

// QC2ComponentStoreFactoryGetter(1, 0) returns a factory pointer
typedef void* (*QC2FactoryGetterFunc)(int, int);
typedef void (*CreateStoreFunc)(std::shared_ptr<C2ComponentStore>*, void*);

int main(int /* argc */, char** /* argv */) {
    LOG(INFO) << "media HW codec (64-bit) service starting...";
    signal(SIGPIPE, SIG_IGN);

    if (access(kBaseSeccompPolicyPath, R_OK) == 0) {
        android::SetUpMinijail(kBaseSeccompPolicyPath, kExtSeccompPolicyPath);
    } else {
        LOG(WARNING) << "Seccomp policy not found at " << kBaseSeccompPolicyPath
                     << ", running without minijail";
    }

    using namespace ::android;
    ProcessState::initWithDriver("/dev/vndbinder");
    ProcessState::self()->startThreadPool();
    hardware::configureRpcThreadpool(8, true /* callerWillJoin */);

    void* handle = dlopen("libqcodec2_core.so", RTLD_NOW);
    if (!handle) {
        LOG(ERROR) << "QC2HALService: failed to open libqcodec2_core.so: " << dlerror();
        return 1;
    }

    std::shared_ptr<C2ComponentStore> store;

    // Method 1: Call QC2ComponentStore::Get() directly
    QC2GetStoreFunc getStore = (QC2GetStoreFunc)dlsym(handle, "_ZN3qc217QC2ComponentStore3GetEv");
    if (getStore) {
        store = getStore();
    }

    // Method 2: Call QC2ComponentStoreFactoryGetter if Get() is not available
    if (!store) {
        QC2FactoryGetterFunc factoryGetter =
                (QC2FactoryGetterFunc)dlsym(handle, "QC2ComponentStoreFactoryGetter");
        if (factoryGetter) {
            void* factory = factoryGetter(1, 0);
            if (factory) {
                void** vtable = *(void***)factory;
                CreateStoreFunc createStore = (CreateStoreFunc)vtable[0];
                createStore(&store, factory);
            }
        }
    }

    if (!store) {
        LOG(ERROR) << "Cannot create Codec2's IComponentStore default service.";
        dlclose(handle);
        return 1;
    }

    LOG(INFO) << "Codec2's IComponentStore default service created: " << store->getName();

    using namespace ::android::hardware::media::c2::V1_0;
    sp<IComponentStore> hidlStore = new utils::ComponentStore(store);
    if (hidlStore == nullptr) {
        LOG(ERROR) << "Cannot create Codec2's IComponentStore utils::ComponentStore wrapper.";
        dlclose(handle);
        return 1;
    }

    if (hidlStore->registerAsService("default") != OK) {
        LOG(ERROR) << "Cannot register Codec2's IComponentStore default service...";
        dlclose(handle);
        return 1;
    }

    LOG(INFO) << "Codec2's IComponentStore default service registered successfully (64-bit).";
    hardware::joinRpcThreadpool();
    dlclose(handle);
    return 0;
}
