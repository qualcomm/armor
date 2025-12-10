// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CUSTOM_DEFS_H
#define CUSTOM_DEFS_H

#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include <functional>
#include <cmath>

namespace custom {

// Type aliases
using StringMap = std::map<std::string, std::string>;
using FloatVector = std::vector<float>;
using StringCallback = std::function<void(const char *)>;

// Processor type enumeration
enum class ProcessorType {
    CPU,
    GPU,
    DSP,
    NPU,
    TPU
};

// Data packet structure for communication
struct DataPacket {
    uint32_t id;
    uint32_t size;
    uint8_t priority;
    std::vector<uint8_t> payload;
    
    DataPacket() : id(0), size(0), priority(0) {}
    
    DataPacket(uint32_t _id, uint32_t _size, uint8_t _priority) 
        : id(_id), size(_size), priority(_priority) {}
    
    bool isValid() const {
        return size > 0 && size == payload.size();
    }
    
    void resize(uint32_t newSize) {
        size = newSize;
        payload.resize(newSize);
    }
};

// Packet processing status codes
enum class PacketStatus {
    Success,
    InvalidFormat,
    Timeout,
    BufferOverflow,
    Incomplete,
    Corrupted
};

// Configuration options
struct ConfigOptions {
    bool enableLogging;
    bool secureMode;
    int compressionLevel;
    std::string outputPath;
};

// Color enumeration
enum class Color {
    Red,
    Green,
    Blue,
    Yellow
};

// Simple Point class
class Point {
private:
    double x;
    double y;

public:
    Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}
    double getX() const { return x; }
    double getY() const { return y; }
    double distance(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx*dx + dy*dy);
    }
};

// Additional structure for alpha parsing
struct AlphaData {
    int id;
    std::string name;
    FloatVector values;
    bool isValid;
    
    AlphaData() : id(0), isValid(false) {}
    
    AlphaData(int _id, const std::string& _name) 
        : id(_id), name(_name), isValid(true) {}
    
    void addValue(float value) {
        values.push_back(value);
    }
    
    float getAverage() const {
        if (values.empty()) return 0.0f;
        
        float sum = 0.0f;
        for (float val : values) {
            sum += val;
        }
        return sum / values.size();
    }
};

// Template class for data processing
template<typename T>
class DataProcessor {
private:
    T data;
    bool processed;
    
public:
    DataProcessor() : processed(false) {}
    
    explicit DataProcessor(const T& _data) : data(_data), processed(false) {}
    
    void setData(const T& _data) {
        data = _data;
        processed = false;
    }
    
    bool process() {
        // Generic processing logic
        processed = true;
        return true;
    }
    
    bool isProcessed() const {
        return processed;
    }
    
    T& getData() {
        return data;
    }
};

// Specialized template for DataPacket
template<>
class DataProcessor<DataPacket> {
private:
    DataPacket packet;
    PacketStatus status;
    bool processed;
    
public:
    DataProcessor() : status(PacketStatus::Incomplete), processed(false) {}
    
    explicit DataProcessor(const DataPacket& _packet) 
        : packet(_packet), status(PacketStatus::Incomplete), processed(false) {}
    
    void setData(const DataPacket& _packet) {
        packet = _packet;
        processed = false;
        status = PacketStatus::Incomplete;
    }
    
    bool process() {
        if (!packet.isValid()) {
            status = PacketStatus::InvalidFormat;
            return false;
        }
        
        // Process the packet
        status = PacketStatus::Success;
        processed = true;
        return true;
    }
    
    PacketStatus getStatus() const {
        return status;
    }
    
    bool isProcessed() const {
        return processed;
    }
    
    DataPacket& getData() {
        return packet;
    }
};

// Utility functions
bool validatePacket(const DataPacket& packet);
PacketStatus processPacket(DataPacket& packet);
std::string getStatusMessage(PacketStatus status);
AlphaData parseAlphaData(const std::string& input);
Point calculateCentroid(const std::vector<Point>& points);

// Constants
constexpr int MAX_PACKET_SIZE = 8192;
constexpr int DEFAULT_TIMEOUT_MS = 5000;
constexpr int MAX_RETRY_COUNT = 3;
constexpr int ALPHA_MAX_VALUES = 100;
constexpr double EPSILON = 0.00001;

} // namespace custom

#endif // CUSTOM_DEFS_H