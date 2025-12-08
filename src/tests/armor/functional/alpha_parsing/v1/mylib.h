// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MYLIB_H
#define MYLIB_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include <map>
#include <functional>
#include "custom_defs.h"

// Global constants outside any namespace
constexpr int GLOBAL_MAX_SIZE = 2000;
constexpr double GLOBAL_PI = 3.14159265358979323846;
constexpr char GLOBAL_VERSION[] = "v2.0.1";

// Global type definitions outside namespace
typedef unsigned int uint32_t;
typedef signed int int32_t;

// Global utility function declarations
bool isValidInput(const std::string& input);
void logMessage(const char* message, int logLevel = 0);

// C compatibility section
#ifdef __cplusplus
extern "C" {
#endif

// Diagnostic enumerations and types from the second file
typedef enum {
    F_DIAG_EVENT_REPORT,
    F_DIAG_HW_ACCELERATION,
    F_DIAG_MULTI_SIM_MASK,
    F_DIAG_DIAGID_BASED_CMD_PKT,
    F_DIAG_DYNAMIC_ATID,
    F_DIAG_DIAGID_BASED_ASYNC_PKT,
    F_DIAG_EXTENDED_LOGGING,
    F_DIAG_SECURE_CHANNEL
} diag_apps_feature_support_def;

extern int diag_use_dev_node;
extern bool filter_enabled;

typedef enum {
    DB_PARSER_STATE_OFF,
    DB_PARSER_STATE_ON,
    DB_PARSER_STATE_LIST,
    DB_PARSER_STATE_OPEN,
    DB_PARSER_STATE_READ,
    DB_PARSER_STATE_CLOSE,
    DB_PARSER_STATE_GUID_DOWNLOADED,
} qsr4_db_file_parser_state;

typedef enum {
    QSR4_INIT,
    QSR4_THREAD_CREATE,
    QSR4_KILL_THREADS,
    QSR4_CLEANUP
} qsr4_init_state;

typedef enum {
    THREADS_KILL,
    THREADS_CLEANUP
} feature_threads_cleanup;

typedef enum {
    FILE_TYPE_QMDL2,
    FILE_TYPE_QDSS,
    NUM_MDLOG_FILE_TYPES
} file_types;

/* enum to handle packet processing status - using custom::PacketStatus for more detailed status */
enum pkt_status {
    PKT_PROCESS_TEST,
    PKT_PROCESS_ONGOING,
    PKT_PROCESS_DONE
};

/* enum defined to identify packets */
typedef enum {
    CMD_RESP = 0,
    LOG_PKT,
    MSG_PKT,
    ENCRYPTED_PKT,
    EVENT_PKT,
    QTRACE_PKT,
    MAX_PKT_SUPPORTED,
} dmux_pkt_supported;

typedef struct {
    int cmd_code;
    int subsys_id;
    int subsys_cmd_code;
} __attribute__ ((packed)) diag_pkt_header_t;

struct diag_callback_tbl_t {
    int inited;
    int (*cb_func_ptr)(unsigned char *, int len, void *context_data);
    void *context_data;
};

struct diag_uart_tbl_t {
    int proc_type;
    int pid;
    int (*cb_func_ptr)(unsigned char *, int len, void *context_data);
    void *context_data;
};

#ifdef __cplusplus
}
#endif

// Global template function outside namespace
template<typename T>
T globalMax(T a, T b) {
    return (a > b) ? a : b;
}

// Global class outside namespace
class GlobalConfig {
private:
    static bool initialized;
    static int configValue;
    
public:
    static void initialize();
    static int getConfigValue();
    static void setConfigValue(int value);
};

// C++ namespace section from the first file
// Import definitions from custom_defs.h
using custom::DataPacket;
using custom::ProcessorType;
using custom::PacketStatus;
using custom::ConfigOptions;
using custom::MAX_PACKET_SIZE;
using custom::DEFAULT_TIMEOUT_MS;
using custom::MAX_RETRY_COUNT;
using custom::AlphaData;
using custom::DataProcessor;
using custom::ALPHA_MAX_VALUES;
using custom::EPSILON;
using custom::Color;
using custom::Point;

// Type aliases
using StringMap = custom::StringMap;
using FloatVector = custom::FloatVector;
using StringCallback = custom::StringCallback;
using PacketProcessor = std::function<bool(const DataPacket&)>;
using StatusCallback = std::function<void(const PacketStatus&)>;
using ConfigHandler = std::function<void(const ConfigOptions&)>;

// Template type aliases
template<typename K, typename V>
using Dictionary = std::map<K, V>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

// Templated variables with templated types
template<typename K, typename V>
inline Dictionary<K, V> empty_dictionary{};

// Constants
constexpr int MAX_SIZE = 100;
constexpr double PI = 3.14159265358979323846;

// Using constants from custom_defs.h
constexpr int PACKET_BUFFER_SIZE = MAX_PACKET_SIZE;
constexpr int TIMEOUT_MS = DEFAULT_TIMEOUT_MS;
constexpr int RETRY_COUNT = MAX_RETRY_COUNT;

// Using Color enumeration from custom_defs.h

struct alpha {
    int a;
    int b;
    float c;
    char* name;
    DataPacket packet;       // Using DataPacket from custom_defs.h
    ProcessorType processor;  // Using ProcessorType from custom_defs.h
    PacketStatus status;     // Using PacketStatus from custom_defs.h
    AlphaData data;          // Using AlphaData from custom_defs.h
};

// Regular function declarations
void printMessage(const std::string& message);
int add(int a, int b);

double calculateArea(double radius);
double calculateArea(double length, double breath);

// Functions using custom_defs.h types
bool validateCustomPacket(const DataPacket& packet);
PacketStatus processCustomPacket(DataPacket& packet);
std::string getCustomStatusMessage(PacketStatus status);
void configureSystem(const ConfigOptions& options);
ProcessorType getOptimalProcessor(const DataPacket& packet);
AlphaData processAlphaData(const std::string& input);
Point findOptimalPoint(const std::vector<Point>& points);

// Function template
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

// Function template with multiple parameters
template<typename T>
auto add(T a) -> decltype(a) {
    return a;
}

// Using Point class from custom_defs.h

// Class template
template<typename T>
struct Container {
private:
    std::vector<T> elements;
    ConfigOptions config;  // Using ConfigOptions from custom_defs.h

public:
    Container() = default;
    void add(const T& element);
    void remove(size_t index, int a);
    T& get(size_t index);
    size_t size() const;
    void setConfig(const ConfigOptions& options) { config = options; }
    ConfigOptions getConfig() const { return config; }
};

// Class template implementation
template<typename T>
void Container<T>::add(const T& element) {
    elements.push_back(element);
}

template<typename T>
void Container<T>::remove(size_t index, int a) {
    if (index < elements.size()) {
        elements.erase(elements.begin() + index);
    }
}

template<typename T>
T& Container<T>::get(size_t index) {
    return elements.at(index);
}

// Template with default parameter
template<typename T = int>
class NumericValue {
private:
    T value;

public:
    NumericValue(T val = T{}) : value(val) {}
    void setValue(T val) { value = val; }
};

// Template specialization
template<>
class Container<bool> {
private:
    std::vector<bool> elements;
    custom::ConfigOptions config;  // Using ConfigOptions from custom_defs.h

public:
    Container() = default;
    size_t size() const;
    void setConfig(const custom::ConfigOptions& options) { config = options; }
    custom::ConfigOptions getConfig() const { return config; }
};

// Templated variable
template<typename T>
constexpr T DEFAULT_VALUE = T{};

// Specialized templated variables
template<>
constexpr int DEFAULT_VALUE<float> = 0;

template<>
constexpr double DEFAULT_VALUE<bool> = 0.0;

template<>
constexpr char DEFAULT_VALUE<int> = '\0';

// Templated variable with multiple template parameters
template<typename K, typename V>
inline Dictionary<K, V> DEFAULT_MAP = {{DEFAULT_VALUE<K>, DEFAULT_VALUE<V>}};

// Class with templated fields
template<typename T, typename U>
class Pair {
public:
    T first;
    U second;
    custom::PacketStatus status;  // Using PacketStatus from custom_defs.h
    
    Pair(T f, U s) : first(f), second(s), status(custom::PacketStatus::Success) {}
    
    // Template method within a template class
    template<typename V>
    V convert() const {
        return static_cast<V>(second);
    }
    
    // Method using custom_defs.h types
    void processWithStatus(custom::DataPacket& packet) {
        // Process packet and update status
        if (packet.isValid()) {
            status = custom::PacketStatus::Success;
        } else {
            status = custom::PacketStatus::InvalidFormat;
        }
    }
};

// Variable template for compile-time type checking
template<typename T>
constexpr int is_numeric_v = std::is_arithmetic<T>::value;

// Template variable that depends on the type's properties
template<typename T>
constexpr int type_size = sizeof(T);

// Class that uses DataProcessor from custom_defs.h
class AlphaProcessor {
private:
    DataProcessor<AlphaData> processor;
    std::vector<AlphaData> processedData;
    
public:
    AlphaProcessor() = default;
    
    void addData(const AlphaData& data) {
        processor.setData(data);
        if (processor.process()) {
            processedData.push_back(processor.getData());
        }
    }
    
    void addData(const std::string& input) {
        AlphaData data = custom::parseAlphaData(input);
        addData(data);
    }
    
    std::vector<AlphaData>& getProcessedData() {
        return processedData;
    }
    
    void clear() {
        processedData.clear();
    }
    
    size_t count() const {
        return processedData.size();
    }
};

// Class that uses DataProcessor with DataPacket
class PacketHandler {
private:
    DataProcessor<DataPacket> processor;
    PacketStatus lastStatus;
    
public:
    PacketHandler() : lastStatus(PacketStatus::Incomplete) {}
    
    bool handlePacket(const DataPacket& packet) {
        processor.setData(packet);
        bool result = processor.process();
        lastStatus = processor.getStatus();
        return result;
    }
    
    PacketStatus getLastStatus() const {
        return lastStatus;
    }
    
    DataPacket& getCurrentPacket() {
        return processor.getData();
    }
};

// Additional global namespace content after mylib namespace
namespace utils {
    // Utility functions
    void initializeSystem();
    bool validateConfig(const std::string& configPath);
    
    // Utility functions using custom_defs.h
    PacketStatus processPacketWithRetry(DataPacket& packet, int maxRetries = MAX_RETRY_COUNT);
    bool isProcessorCompatible(const ProcessorType& processor, const DataPacket& packet);
    ConfigOptions loadDefaultConfig();
    AlphaData mergeAlphaData(const AlphaData& a, const AlphaData& b);
    std::vector<Point> generatePointGrid(int rows, int cols, double spacing = 1.0);
    
    // Utility template
    template<typename T>
    class Optional {
    private:
        bool hasValue;
        T value;
        
    public:
        Optional() : hasValue(false) {}
        Optional(const T& val) : hasValue(true), value(val) {}
        
        bool isPresent() const { return hasValue; }
        T getValue() const { return value; }
    };
}

// Global template class outside any namespace
template<typename T>
class GlobalContainer {
private:
    static std::vector<T> items;
    static ConfigOptions globalConfig;  // Using ConfigOptions from custom_defs.h
    
public:
    static void add(const T& item) {
        items.push_back(item);
    }
    
    static size_t count() {
        return items.size();
    }
    
    static void setGlobalConfig(const ConfigOptions& config) {
        globalConfig = config;
    }
    
    static ConfigOptions getGlobalConfig() {
        return globalConfig;
    }
};

// Initialize static members
template<typename T>
std::vector<T> GlobalContainer<T>::items;

template<typename T>
custom::ConfigOptions GlobalContainer<T>::globalConfig = {false, false, 0, ""};

#endif // MYLIB_H