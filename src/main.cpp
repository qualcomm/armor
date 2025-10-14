// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "options_handler.hpp"

int main(int argc, const char **argv) {
    if (!handleCommandLineOptions(argc, argv)) {
        return 1;
    }
    return 0;
}
