// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef GIT_UTILS_HPP
#define GIT_UTILS_HPP

#include <string>
#include <vector>

bool isGitRepo(const std::string& path);
std::string getGitRepoRoot(const std::string& path);
bool hasUncommittedChanges(const std::string& repoRoot);
std::string getRefInfo(const std::string& repoRoot, const std::string& gitRef);
std::vector<std::string> getChangedHeaders(const std::string& repoRoot,
                                            const std::string& oldRef,
                                            const std::string& newRef);
std::string createGitWorktree(const std::string& repoRoot, const std::string& gitRef);
void removeGitWorktree(const std::string& repoRoot, const std::string& worktreePath);

struct GitWorktreeGuard {
    std::string repoRoot;
    std::string worktreePath;
    bool active = false;

    GitWorktreeGuard() = default;
    GitWorktreeGuard(const GitWorktreeGuard&) = delete;
    GitWorktreeGuard& operator=(const GitWorktreeGuard&) = delete;

    ~GitWorktreeGuard() {
        if (active && !worktreePath.empty())
            removeGitWorktree(repoRoot, worktreePath);
    }
};

#endif
