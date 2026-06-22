// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#include "git_utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <string>
#include <vector>
#include <filesystem>

static std::string readPipe(FILE* pipe) {
    std::string result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    if (pclose(pipe) < 0)
        return "";
    if (!result.empty() && result.back() == '\n')
        result.pop_back();
    return result;
}

bool isGitRepo(const std::string& path) {
    std::string cmd = "git -C \"" + path + "\" rev-parse --is-inside-work-tree > /dev/null 2>&1";
    return std::system(cmd.c_str()) == 0;
}

std::string getGitRepoRoot(const std::string& path) {
    std::string cmd = "git -C \"" + path + "\" rev-parse --show-toplevel 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    return readPipe(pipe);
}

bool hasUncommittedChanges(const std::string& repoRoot) {
    // exits 0 if clean, non-zero if there are staged or unstaged changes vs HEAD
    std::string cmd = "git -C \"" + repoRoot + "\" diff --quiet HEAD 2>/dev/null";
    return std::system(cmd.c_str()) != 0;
}

std::string getRefInfo(const std::string& repoRoot, const std::string& gitRef) {
    std::string cmd = "git -C \"" + repoRoot + "\" log -1 --format=\"%h, %ar\" " + gitRef + " 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    return readPipe(pipe);
}

std::vector<std::string> getChangedHeaders(const std::string& repoRoot,
                                            const std::string& oldRef,
                                            const std::string& newRef) {
    std::string cmd = "git -C \"" + repoRoot + "\" diff --name-only " + oldRef;
    if (!newRef.empty())
        cmd += " " + newRef;
    cmd += " 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return {};
    std::vector<std::string> result;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        if (!line.empty() && line.back() == '\n') line.pop_back();
        if (line.empty()) continue;
        auto ext = std::filesystem::path(line).extension().string();
        if (ext == ".h" || ext == ".hpp" || ext == ".hxx")
            result.push_back(line);
    }
    if (pclose(pipe) < 0)
        return {};
    return result;
}

std::string createGitWorktree(const std::string& repoRoot, const std::string& gitRef) {
    static int counter = 0;
    std::string tempPath = "/tmp/armor_worktree_"
        + std::to_string(getpid())
        + "_"
        + std::to_string(std::time(nullptr))
        + "_"
        + std::to_string(++counter);

    std::string cmd = "git -C \"" + repoRoot + "\" worktree add --detach \"" + tempPath + "\" " + gitRef + " > /dev/null 2>&1";
    if (std::system(cmd.c_str()) != 0)
        return "";
    return tempPath;
}

void removeGitWorktree(const std::string& repoRoot, const std::string& worktreePath) {
    std::string cmd = "git -C \"" + repoRoot + "\" worktree remove --force \"" + worktreePath + "\" 2>/dev/null";
    std::system(cmd.c_str());
}
