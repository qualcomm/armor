#!/bin/bash

# Usage message
usage() {
    echo "Usage: $0 [--package]"
    echo ""
    echo "Options:"
    echo "  --package             Generate the Debian package after building"
    echo "  --help                Show this help message"
    echo ""
    echo "Default behaviour:      Build the binary only"
    exit 1
}

# Check for help flag
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    usage
fi

PACKAGE=false

# Parse arguments
if [[ "$1" == "--package" ]]; then
    PACKAGE=true
    shift
    if [[ "$#" -ne 0 ]]; then
        echo "Error: --package does not take any additional arguments."
        usage
    fi
elif [[ "$#" -ne 0 ]]; then
    echo "Error: Invalid arguments."
    usage
fi

BUILD_DIR="build"
BINARY_NAME="armor"
BINARY_PATH="$BUILD_DIR/$BINARY_NAME"

build_binary() {
    echo "🔧 Building binary..."
    rm -rf "$BUILD_DIR"
    cmake -S . -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_COMPILER=/usr/bin/g++-11
    cmake --build "$BUILD_DIR" --parallel

    if [ -f "$BINARY_PATH" ]; then
        echo "✅ Binary built successfully at $BINARY_PATH"
    else
        echo "❌ Error: Binary not found after build."
        exit 1
    fi
}

# Check if binary exists
if [ ! -f "$BINARY_PATH" ]; then
    echo "⚠️ Binary not found at $BINARY_PATH. Building now..."
    build_binary
else
    echo "✅ Binary already exists at $BINARY_PATH."
fi

# Package if requested
if $PACKAGE; then
    echo "📦 Packaging with CPack..."
    cpack --config "$BUILD_DIR/CPackConfig.cmake"
fi