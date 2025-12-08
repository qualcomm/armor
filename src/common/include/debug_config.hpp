// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <llvm/Support/raw_ostream.h>

class DebugConfig {
public:
    enum class Level {
        ERROR = 0,
        LOG   = 1,
        INFO  = 2,
        DEBUG = 3
    };

    static DebugConfig& instance() {
        static DebugConfig inst;
        return inst;
    }

    void setLevel(Level lvl) { logLevel = lvl; }
    Level getLevel() const { return logLevel; }

    //allow injecting a shared sink (not owned). Thread-safe.
    void setSink(llvm::raw_ostream* sink) {
        std::scoped_lock<std::mutex> lock(mu_);
        sink_ = sink;
    }

    //reuse the same sink (e.g., session.cpp for clang diagnostics)
    llvm::raw_ostream* getSink() const {
        std::scoped_lock<std::mutex> lock(mu_);
        return sink_;
    }

    void log(const std::string& msg, Level lvl = Level::DEBUG) const {
        if (static_cast<int>(lvl) <= static_cast<int>(logLevel)) {
            std::scoped_lock<std::mutex> lock(mu_);
            if (sink_) {
                // Preferred: single shared stream (no cross-buffering)
                *sink_ << "[" << levelToString(lvl) << "] " << msg << "\n";
                sink_->flush();
                return;
            }

            // Fallback: original behavior (per-call file open)
            const std::string logDir  = "debug_output/logs";
            const std::string logPath = logDir + "/diagnostics.log";
            std::filesystem::create_directories(logDir);

            std::ofstream logFile(logPath, std::ios::app);
            if (logFile.is_open()) {
                logFile << "[" << levelToString(lvl) << "] " << msg << "\n";
            } else {
                llvm::errs() << "[ERROR] Failed to open log file: " << logPath << "\n";
            }
        }
    }

private:
    DebugConfig() : logLevel(Level::LOG), sink_(nullptr) {}
    Level logLevel;

    //shared sink pointer (not owned)
    mutable std::mutex mu_;
    llvm::raw_ostream* sink_;

    static const char* levelToString(Level lvl) {
        switch (lvl) {
            case Level::ERROR: return "ERROR";
            case Level::LOG:   return "LOG";
            case Level::INFO:  return "INFO";
            case Level::DEBUG: return "DEBUG";
            default:           return "UNKNOWN";
        }
    }

    DebugConfig(const DebugConfig&) = delete;
    DebugConfig& operator=(const DebugConfig&) = delete;
};
