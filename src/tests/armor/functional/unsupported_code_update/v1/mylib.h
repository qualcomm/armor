// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef MYLIB_H
#define MYLIB_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>

// Forward declarations
namespace data {
    class DataProcessor;
    template<typename T>
    class Container;
}

namespace utils {
    class Logger;
    struct Config;
}

// Namespace: utils
namespace utils {

// Configuration struct
struct Config {
    std::string name;
    int version;
    bool enabled;
    
    Config() : name("default"), version(1), enabled(true) {}
    Config(const std::string& n, int v, bool e) 
        : name(n), version(v), enabled(e) {}
};

// Logger class with inline implementation
class Logger {
private:
    std::string prefix;
    bool verbose;
    
public:
    Logger(const std::string& p = "LOG") : prefix(p), verbose(false) {}
    
    void setVerbose(bool v) { verbose = v; }
    
    void log(const std::string& message) const {
        std::cout << "[" << prefix << "] " << message << std::endl;
    }
    
    template<typename T>
    void logValue(const std::string& name, const T& value) const {
        std::cout << "[" << prefix << "] " << name << ": " << value << std::endl;
    }
};

// Template function for type conversion
template<typename To, typename From>
inline To convert(const From& value) {
    return static_cast<To>(value);
}

// Template specialization
template<>
inline std::string convert<std::string, int>(const int& value) {
    return std::to_string(value);
}

} // namespace utils

// Namespace: data
namespace data {

// Base class for data items
class DataItem {
protected:
    std::string id;
    int timestamp;
    
public:
    DataItem(const std::string& itemId) : id(itemId), timestamp(0) {}
    virtual ~DataItem() = default;
    
    virtual std::string getType() const = 0;
    virtual void process() = 0;
    
    std::string getId() const { return id; }
    void setTimestamp(int ts) { timestamp = ts; }
};

// Derived class
class TextData : public DataItem {
private:
    std::string content;
    
public:
    TextData(const std::string& id, const std::string& text) 
        : DataItem(id), content(text) {}
    
    std::string getType() const override { return "TextData"; }
    
    void process() override {
        // Simple processing
        content = "[PROCESSED] " + content;
    }
    
    std::string getContent() const { return content; }
};

// Template container class
template<typename T>
class Container {
private:
    std::vector<T> items;
    std::string name;
    
public:
    Container(const std::string& containerName) : name(containerName) {}
    
    void add(const T& item) {
        items.push_back(item);
    }
    
    size_t size() const {
        return items.size();
    }
    
    T& get(size_t index) {
        return items.at(index);
    }
    
    const T& get(size_t index) const {
        return items.at(index);
    }
    
    void clear() {
        items.clear();
    }
    
    template<typename Func>
    void forEach(Func func) {
        for (auto& item : items) {
            func(item);
        }
    }
};

// Template specialization for pointers
template<typename T>
class Container<T*> {
private:
    std::vector<T*> items;
    bool ownsPointers;
    
public:
    Container(bool owns = false) : ownsPointers(owns) {}
    
    ~Container() {
        if (ownsPointers) {
            for (auto* ptr : items) {
                delete ptr;
            }
        }
    }
    
    void add(T* item) {
        items.push_back(item);
    }
    
    size_t size() const {
        return items.size();
    }
};

// DataProcessor class
class DataProcessor {
private:
    utils::Logger logger;
    utils::Config config;
    Container<std::shared_ptr<DataItem>> dataItems;
    
public:
    DataProcessor() : logger("DataProcessor"), dataItems("MainContainer") {}
    
    void setConfig(const utils::Config& cfg) {
        config = cfg;
        logger.log("Configuration updated: " + config.name);
    }
    
    void addItem(std::shared_ptr<DataItem> item) {
        dataItems.add(item);
        logger.log("Added item: " + item->getId());
    }
    
    void processAll() {
        logger.log("Processing all items...");
        dataItems.forEach([this](std::shared_ptr<DataItem>& item) {
            item->process();
            logger.log("Processed: " + item->getId());
        });
    }
    
    size_t getItemCount() const {
        return dataItems.size();
    }
};

} // namespace data

// Global template function
template<typename T>
T max(const T& a, const T& b) {
    return (a > b) ? a : b;
}

// Function template with multiple parameters
template<typename T, typename U>
auto multiply(const T& a, const U& b) -> decltype(a * b) {
    return a * b;
}

// Variadic template function
template<typename T>
T sum(T value) {
    return value;
}

template<typename T, typename... Args>
T sum(T first, Args... args) {
    return first + sum(args...);
}

// Enum class
enum class Status {
    Idle,
    Running,
    Paused,
    Completed,
    Error
};

// Helper function
inline const char* statusToString(Status status) {
    switch (status) {
        case Status::Idle: return "Idle";
        case Status::Running: return "Running";
        case Status::Paused: return "Paused";
        case Status::Completed: return "Completed";
        case Status::Error: return "Error";
        default: return "Unknown";
    }
}

#endif // MYLIB_H