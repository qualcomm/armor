// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef MYLIB_H
#define MYLIB_H

#include <mutex>
#include <shared_mutex>
#include <string>

struct OfflineLogger {

    OfflineLogger() = default;

    ~OfflineLogger() = default;

    void logMessage(const std::string& message);

    static std::mutex lock;
    static std::shared_mutex threadLock; 
    bool isClosed = false;
};

#endif