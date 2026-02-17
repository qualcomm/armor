// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include <string>
#include <string_view>
#include <filesystem>
#include <mutex>
#include <memory>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Path.h>
#include <utility>

#include "comm_def.hpp"

class BaseLogStream;

class DebugConfig {
public:

enum class Level {
        NONE = 0,
        ERROR = 2,
        WARNING = 3,
        INFO = 4,
        DEBUG = 5
    };

enum class ConsoleOption{
        NONE = 0,
        ERROR = 1,
        INFO = 2
    };

    class LogStream;
    class TestLogStream;

    static DebugConfig& getInstance() {
        static DebugConfig inst;
        return inst;
    }

    bool initialize(llvm::StringRef logFilePath = LOG_FILE_PATH) {
        std::scoped_lock<std::mutex> lock(mutex);
        
        if (isInitialized.load(std::memory_order_relaxed)) {
            return true;
        }

        llvm::SmallString<256> logPath(logFilePath);
        llvm::StringRef parentDir = llvm::sys::path::parent_path(logPath);
        if (!parentDir.empty()) {
            std::filesystem::create_directories(parentDir.str());
        }
        
        std::error_code ec;
        fileStream = std::make_unique<llvm::raw_fd_ostream>(
            logFilePath, ec, 
            llvm::sys::fs::OF_Text | llvm::sys::fs::OF_Append
        );
        
        if (ec) {
            activeStream = &llvm::errs();
            return false;
        }
        
        activeStream = fileStream.get();

        #ifdef TESTING_ENABLED
            std::error_code debugEc;
            debugFileStream = std::make_unique<llvm::raw_fd_ostream>(
                TEST_LOG_FILE_PATH, debugEc, 
                llvm::sys::fs::OF_Text
            );
            
            if (debugEc) {
                debugStream = &llvm::errs();
                return false;
            }
    
            debugStream = debugFileStream.get();
        #endif
        
        isInitialized.store(true, std::memory_order_relaxed);
        return true;
    }

    void setLevel(Level lvl) { 
        logLevel.store(lvl, std::memory_order_relaxed);
    }
    
    Level getLevel() const { 
        return logLevel.load(std::memory_order_relaxed);
    }

    void setSink(llvm::raw_ostream* sink) {
        std::scoped_lock<std::mutex> lock(mutex);
        externalSink = sink;
    }

    llvm::raw_ostream* getSink() const {
        std::scoped_lock<std::mutex> lock(mutex);
        return externalSink ? externalSink : activeStream;
    }

    LogStream getStream(Level lvl) const;

    LogStream getConsoleAndStream(Level lvl, ConsoleOption consoleOption) const;

    #ifdef TESTING_ENABLED
    TestLogStream getTestStream() const;
    #endif

    void flush() const {
        std::scoped_lock<std::mutex> lock(mutex);
        if (activeStream) {
            activeStream->flush();
        }
    }

    ~DebugConfig() {
        flush();
    }

private:

    friend class LogStream;
    friend class TestLogStream;

    DebugConfig() : logLevel(Level::INFO), isInitialized(false), 
                   activeStream(&llvm::errs()), externalSink(nullptr) {}

    constexpr llvm::StringRef levelToString(Level lvl) const {
        switch (lvl) {
            case Level::ERROR:   return "ERROR";
            case Level::WARNING: return "WARN";
            case Level::INFO:    return "INFO";
            case Level::DEBUG:   return "DEBUG";
            default:             return "UNKN";
        }
    }

    constexpr llvm::StringRef consoleOptionToString(ConsoleOption option) const {
        switch (option) {
            case ConsoleOption::ERROR:   return "ERROR";
            case ConsoleOption::INFO:    return "INFO";
            default:             return "UNKN";
        }
    }

    mutable std::mutex mutex;
    std::atomic<Level> logLevel;
    std::atomic<bool> isInitialized;
    std::unique_ptr<llvm::raw_fd_ostream> fileStream;
    llvm::raw_ostream* activeStream;
    #ifdef TESTING_ENABLED
        std::unique_ptr<llvm::raw_fd_ostream> debugFileStream;
        llvm::raw_ostream* debugStream;
    #endif
    llvm::raw_ostream* externalSink;

    DebugConfig(const DebugConfig&) = delete;
    DebugConfig& operator=(const DebugConfig&) = delete;
};

class BaseLogStream {
protected:
    explicit BaseLogStream(const DebugConfig& config) 
        : config(config), OS(bufferStorage), consoleOS(consoleBufferStorage), isActive(true), isConsoleActive(false) {}

    const DebugConfig& config;
    bool isActive;
    bool isConsoleActive;
    llvm::SmallString<512> bufferStorage;
    llvm::SmallString<512> consoleBufferStorage;
    llvm::raw_svector_ostream OS;
    llvm::raw_svector_ostream consoleOS;

    BaseLogStream(const BaseLogStream&) = delete;
    BaseLogStream& operator=(const BaseLogStream&) = delete;
    BaseLogStream& operator=(BaseLogStream&&) = delete;

public:
    virtual ~BaseLogStream() = default;

    BaseLogStream(BaseLogStream&& other) noexcept
        : config(other.config), 
          isActive(other.isActive), 
          bufferStorage(std::move(other.bufferStorage)),
          consoleBufferStorage(std::move(other.consoleBufferStorage)),
          consoleOS(consoleBufferStorage),
          OS(bufferStorage) {
        other.isActive = false;
        other.isConsoleActive = false;
    }

    template<typename T>
    BaseLogStream& operator<<(const T& value) {
        if (isActive) {
            OS << value;
        }
        if(isConsoleActive){
            consoleOS << value;
        }
        return *this;
    }

    BaseLogStream& operator<<(llvm::StringRef str) {
        if (isActive) {
            OS << str;
        }
        if(isConsoleActive){
            consoleOS << str;
        }
        return *this;
    }

    BaseLogStream& operator<<(std::string_view str) {
        if (isActive) {
            OS << str;
        }
        if(isConsoleActive){
            consoleOS << str;
        }
        return *this;
    }

    BaseLogStream& operator<<(const char* str) {
        if (isActive) {
            OS << str;
        }
        if(isConsoleActive){
            consoleOS << str;
        }
        return *this;
    }

    BaseLogStream& operator<<(const std::string& str) {
        if (isActive) {
            OS << str;
        }
        if(isConsoleActive){
            consoleOS << str;
        }
        return *this;
    }
};

class DebugConfig::LogStream : public BaseLogStream {
public:
    explicit LogStream(const DebugConfig& config, Level lvl, ConsoleOption consoleOption = ConsoleOption::NONE) 
        : BaseLogStream(config), level(lvl), consoleOption(consoleOption) {
        if(lvl == Level::NONE){
            isActive = false;
        }
        else{
            isActive = static_cast<int>(lvl) <= static_cast<int>(config.logLevel.load(std::memory_order_relaxed));
            if (isActive){
                OS << "[" << config.levelToString(lvl) << "] ";
            }
        }
        if(consoleOption == ConsoleOption::NONE){
            isConsoleActive = false;
        }
        else{
            isConsoleActive = true;
            consoleOS << "[" << config.consoleOptionToString(consoleOption) << "] ";
        }
    }

    ~LogStream() override {
        bool shouldLogToFile = isActive && !bufferStorage.empty() && config.activeStream;
        bool shouldLogToConsole = isConsoleActive && !consoleBufferStorage.empty();
        if (shouldLogToConsole || shouldLogToFile) {
            std::scoped_lock<std::mutex> lock(config.mutex);
            if (shouldLogToFile) {
                *config.activeStream << bufferStorage;
                config.activeStream->flush();
            }
            if(shouldLogToConsole){
                switch (consoleOption) {
                    case DebugConfig::ConsoleOption::INFO:
                        llvm::outs() << consoleBufferStorage;
                        llvm::outs().flush();
                        break;
                    case DebugConfig::ConsoleOption::ERROR:
                        llvm::errs() << consoleBufferStorage;
                        llvm::errs().flush();
                        break;
                    case DebugConfig::ConsoleOption::NONE:
                    default:
                        break;
                }
            }
        }
    }

    LogStream(LogStream&& other) noexcept
        : BaseLogStream(std::move(other)), level(other.level), consoleOption(other.consoleOption) {}

private:
    Level level;
    ConsoleOption consoleOption;
};

inline DebugConfig::LogStream DebugConfig::getStream(Level lvl) const {
    return LogStream(*this, lvl);
}

inline DebugConfig::LogStream DebugConfig::getConsoleAndStream(Level lvl, ConsoleOption consoleOption) const{
    return LogStream(*this, lvl, consoleOption);
}

#ifdef TESTING_ENABLED
    class DebugConfig::TestLogStream : public BaseLogStream {
    public:
        explicit TestLogStream(const DebugConfig& config) 
            : BaseLogStream(config) {}

        ~TestLogStream() override {
            if (!bufferStorage.empty()) {
                std::scoped_lock<std::mutex> lock(config.mutex);
                if (config.debugStream) {
                    *config.debugStream << bufferStorage;
                    config.debugStream->flush();
                }
            }
        }
    };

    inline DebugConfig::TestLogStream DebugConfig::getTestStream() const {
        return TestLogStream(*this);
    }
#endif

namespace armor {

    #ifdef TESTING_ENABLED
        inline DebugConfig::TestLogStream test() {
            return DebugConfig::getInstance().getTestStream();
        }
    #else
        inline DebugConfig::LogStream test() {
            return DebugConfig::getInstance().getStream(DebugConfig::Level::NONE);
        }
    #endif

    inline DebugConfig::LogStream debug() {
        return DebugConfig::getInstance().getStream(DebugConfig::Level::DEBUG);
    }

    inline DebugConfig::LogStream info() {
        return DebugConfig::getInstance().getStream(DebugConfig::Level::INFO);
    }

    inline DebugConfig::LogStream warning() {
        return DebugConfig::getInstance().getStream(DebugConfig::Level::WARNING);
    }

    inline DebugConfig::LogStream error() {
        return DebugConfig::getInstance().getStream(DebugConfig::Level::ERROR);
    }

    inline DebugConfig::LogStream user_print() {
        return DebugConfig::getInstance().getConsoleAndStream(
            DebugConfig::Level::INFO, 
            DebugConfig::ConsoleOption::INFO
        );
    }

    inline DebugConfig::LogStream user_error() {
        return DebugConfig::getInstance().getConsoleAndStream(
            DebugConfig::Level::ERROR,
            DebugConfig::ConsoleOption::ERROR
        );
    }

    inline DebugConfig::LogStream warn() { return warning(); }
    
    inline DebugConfig::LogStream err() { return error(); }

}

#define TEST_LOG armor::test()