// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once
#include <iostream>
#include "llvm/Support/raw_ostream.h"
#include "debug_config.hpp"

// Prints to terminal AND logs to file
#define USER_PRINT(msg) \
    do { \
        std::cout << msg << std::endl; \
        DebugConfig::instance().log(msg, DebugConfig::Level::INFO); \
    } while(0)

#define USER_ERROR(msg) \
    do { \
        llvm::errs() << msg << "\n"; \
        DebugConfig::instance().log(msg, DebugConfig::Level::ERROR); \
    } while(0)
