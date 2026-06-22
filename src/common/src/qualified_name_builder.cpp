// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "qualified_name_builder.hpp"
#include "logger.hpp"
#include <llvm-14/llvm/ADT/StringRef.h>

void StringBuilder::push(llvm::StringRef Name) {
    offsets.push_back(buffer.size());
    if (!buffer.empty()) buffer += "::";
    buffer += Name;
}

void StringBuilder::pop() {
    armor::debug()<< "Qualified Name : " << buffer << "\n";
    if (!offsets.empty()) {
        buffer.resize(offsets.back());
        offsets.pop_back();
    }
}

llvm::StringRef StringBuilder::get() const {
    return llvm::StringRef(buffer.data(), buffer.size());
}

std::string StringBuilder::getAsString() const {
    return std::string(get());
}

void StringBuilder::overridePush(llvm::StringRef newName, llvm::StringRef usr) {

    armor::debug() << "Override Push \n";

    SavedState saved;
    saved.buffer = buffer;
    saved.offsets = offsets;
    contextMap.insert_or_assign(usr, saved);

    buffer.clear();
    offsets.clear();
    offsets.push_back(0);
    buffer += newName;
    armor::debug() << usr << "\n";
}

void StringBuilder::overridePop(llvm::StringRef usr) {
    auto itr = contextMap.find(usr);
    if ( itr == contextMap.end() ) return;

    buffer.clear();
    offsets.clear();

    SavedState state = itr->getValue();
    contextMap.erase(usr);

    buffer = state.buffer;
    offsets = state.offsets;
    armor::debug() << usr << "\n";
}

bool StringBuilder::isOverrideActive(llvm::StringRef usr){
    return contextMap.find(usr) != contextMap.end();
}
