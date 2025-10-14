// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "qualified_name_builder.hpp"

void QualifiedNameBuilder::push(llvm::StringRef Name) {
    offsets.push_back(buffer.size());
    if (!buffer.empty()) buffer += "::";
    buffer += Name;
}

void QualifiedNameBuilder::pop() {
    if (!offsets.empty()) {
        buffer.resize(offsets.back());
        offsets.pop_back();
    }
}

llvm::StringRef QualifiedNameBuilder::get() const {
    return llvm::StringRef(buffer.data(), buffer.size());
}

std::string QualifiedNameBuilder::getAsString() const {
    return std::string(get());
}
