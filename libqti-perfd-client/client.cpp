/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "libqti-perfd-client"

#include <stdint.h>
#include <log/log.h>
#include <functional>

extern "C" {

void perf_event(int, char*, int, int*) {}
void perf_event_offload(int, char*, int, int*) {}

int perf_get_feedback(int, char*) {
    return 233;
}

int perf_get_feedback_extn(int, char*, unsigned int, char*) {
    return 233;
}

int perf_get_prop(const char*, char*) {
    return 0;
}

void perf_hint(int, const char*, int, int) {}
void perf_hint_offload(int, const char*, int, int, int, int, int[]) {}

int perf_hint_renew(int, int, const char*, int, int, int, int[]) {
    return 233;
}

int perf_lock_acq(int handle, int, int[], int) {
    return handle > 0 ? handle : 233;
}

int perf_lock_acq_rel(int handle, int, int[], int, int) {
    return handle > 0 ? handle : 233;
}

void perf_lock_cmd(int) {}

int perf_lock_rel(int handle) {
    return handle > 0 ? handle : 233;
}

int perf_lock_use_profile(int handle, int) {
    return handle > 0 ? handle : 233;
}

int perf_sync_request(int) {
    return 0;
}

void perf_wait_get_prop(const char*, char*) {}

} // extern "C"

class PerfThreadPool {
public:
    static PerfThreadPool singleton_pool_obj;
    PerfThreadPool();
    ~PerfThreadPool();
    int create(int);
    int placeTask(std::function<void()>&&);
    int placeTaskAt(std::function<void()>&&, int);
    int resize(int);
    void setNonRT();
    int getDedicated();
    static void executeTask(void*);
};

PerfThreadPool PerfThreadPool::singleton_pool_obj;

PerfThreadPool::PerfThreadPool() {}
PerfThreadPool::~PerfThreadPool() {}

int PerfThreadPool::create(int) {
    return 0;
}

int PerfThreadPool::placeTask(std::function<void()>&& task) {
    if (task) {
        task();
    }
    return 0;
}

int PerfThreadPool::placeTaskAt(std::function<void()>&& task, int) {
    if (task) {
        task();
    }
    return 0;
}

int PerfThreadPool::resize(int) {
    return 0;
}

void PerfThreadPool::setNonRT() {}

int PerfThreadPool::getDedicated() {
    return 0;
}

void PerfThreadPool::executeTask(void*) {}
