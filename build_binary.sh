# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#!/bin/bash

# Usage message
usage() {
    echo "Usage: $0 [--build]"
    echo ""
    echo "Options:"
    echo "  --build               Force rebuild the binary"
    echo "  --help                Show this help message"
    echo ""
    echo "Default behaviour:      Use existing binary if available, otherwise build"
    exit 1
}

# Check for help flag
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    usage
fi

FORCE_BUILD=false

# Parse arguments
if [[ "$1" == "--build" ]]; then
    FORCE_BUILD=true
    shift
    if [[ "$#" -ne 0 ]]; then
        echo "Error: --build does not take any additional arguments."
        usage
    fi
elif [[ "$#" -ne 0 ]]; then
    echo "Error: Invalid arguments."
    usage
fi

BUILD_DIR="build"
BINARY_PATH="$BUILD_DIR/src/armor/armor"

build_binary() {
    echo "🔧 Building binary..."
    rm -rf "$BUILD_DIR"
    cmake -S . -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_COMPILER=/usr/bin/g++-11
    cmake --build "$BUILD_DIR" --parallel $(nproc)

    if [ -f "$BINARY_PATH" ]; then
        echo "✅ Binary built successfully at $BINARY_PATH"
    else
        echo "❌ Error: Binary not found after build."
        exit 1
    fi
}

# Check if binary exists or force build
if $FORCE_BUILD; then
    echo "🔄 Force rebuild requested..."
    build_binary
elif [ ! -f "$BINARY_PATH" ]; then
    echo "⚠️ Binary not found at $BINARY_PATH. Building now..."
    build_binary
else
    echo "✅ Binary already exists at $BINARY_PATH."
fi
