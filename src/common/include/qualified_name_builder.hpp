// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"

class StringBuilder {
public:
    void push(llvm::StringRef Name);
    void pop();
    llvm::StringRef get() const;
    std::string getAsString() const;

    void overridePush(llvm::StringRef newName, llvm::StringRef usr);
    void overridePop(llvm::StringRef usr);
    bool isOverrideActive(llvm::StringRef usr);

private:
    llvm::SmallString<256> buffer;
    llvm::SmallVector<size_t, 16> offsets;

    struct SavedState {
        llvm::SmallString<256> buffer;
        llvm::SmallVector<size_t, 16> offsets;
    };
    llvm::StringMap<SavedState> contextMap;
};
