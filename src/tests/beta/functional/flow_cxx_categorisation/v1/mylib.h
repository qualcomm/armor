// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef LIB_H
#define LIB_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <cmath>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

class Vehicle;
class Engine;
template<typename T> class Node;
template<typename T, typename U> class Graph;
struct Vertex;
enum class Status;
union Data;

// ============================================================================
// ENUMERATIONS
// ============================================================================

enum class Color { RED, GREEN, BLUE };
enum class LogLevel : int { DEBUG = 0, INFO = 1, ERROR = 2 };
enum FilePermission { READ = 1 << 0, WRITE = 1 << 1, EXECUTE = 1 << 2 };
enum class Status { IDLE, RUNNING, STOPPED };
enum NetworkProtocol { TCP = 0, UDP = 1, HTTP = 2 };

// ============================================================================
// STRUCTURES
// ============================================================================

struct Point2D {
    enum class Status { IDLE, RUNNING, STOPPED };
    double x, y;
    Point2D() : x(0.0), y(0.0) {}
    Point2D(double x, double y) : x(x), y(y) {}
    double distance() const { return std::sqrt(x * x + y * y); }
};

struct Vertex {
    int id;
    double x, y, z;
    Vertex() : id(0), x(0), y(0), z(0) {}
};

// ============================================================================
// UNIONS
// ============================================================================

union Data {
    int i;
    float f;
    double d;
    Data() : i(0) {}
};

// ============================================================================
// CLASSES WITH NESTED TYPES (FORWARD DECLARED)
// ============================================================================

class Vehicle {
public:
    struct Specifications;
    enum class FuelType;
    union EngineData;
    template<typename T> class Component;
    
    Vehicle();
    ~Vehicle();
    void start();
    double getSpeed() const;
    template<typename T> void addComponent(const T& component);
    static int getVehicleCount();
    
private:
    std::string make;
    double speed;
    static int vehicleCount;
};

// Nested type definitions (outside class)
struct Vehicle::Specifications {
    int year;
    double engineSize;
    typedef int a;
    Specifications() : year(2020), engineSize(2.0) {}
    union alpha{
        int a;
        alpha() {}
    };
};

enum class Vehicle::FuelType { GASOLINE, DIESEL, ELECTRIC };

union Vehicle::EngineData {
    int cylinderCount;
    double batteryCapacity;
    EngineData() : cylinderCount(4) {}
};

template<typename T>
class Vehicle::Component {
private:
    T data;
public:
    Component() : data(T()) {}
    T getData() const { return data; }
};

// ============================================================================
// TEMPLATE CLASS WITH NESTED TYPES
// ============================================================================

template<typename T>
class Node {
public:
    struct NodeData;
    enum class NodeType;
    template<typename U> class ChildNode;
    
    Node();
    T getValue() const;
    
private:
    T value;
    NodeType type;
};

template<typename T>
struct Node<T>::NodeData {
    T data;
    int depth;
    NodeData() : depth(0) {}
};

template<typename T>
enum class Node<T>::NodeType { LEAF, INTERNAL, ROOT };

template<typename T>
template<typename U>
class Node<T>::ChildNode {
private:
    T parentValue;
    U childValue;
public:
    ChildNode() : parentValue(T()), childValue(U()) {}
    T getParentValue() const { return parentValue; }
};

// ============================================================================
// MULTI-TEMPLATE CLASS WITH NESTED TYPES
// ============================================================================

template<typename T, typename U>
class Graph {
public:
    struct GraphNode;
    enum class TraversalType;
    template<typename V> class Iterator;
    
    Graph();
    void addNode(const T& nodeData);
    
private:
    std::vector<GraphNode> nodes;
};

template<typename T, typename U>
struct Graph<T, U>::GraphNode {
    T data;
    int id;
    GraphNode() : id(0) {}
};

template<typename T, typename U>
enum class Graph<T, U>::TraversalType { DEPTH_FIRST, BREADTH_FIRST };

template<typename T, typename U>
template<typename V>
class Graph<T, U>::Iterator {
private:
    V* current;
public:
    Iterator() : current(nullptr) {}
    V& operator*() { return *current; }
};

// ============================================================================
// STRUCT WITH NESTED TYPES
// ============================================================================

struct ComplexStructure {
    enum class Mode { READ_ONLY, WRITE_ONLY };
    
    struct Inner {
        int value;
        Inner() : value(0) {}
    };
    
    template<typename T>
    struct Container {
        T data;
        Container() : data(T()) {}
    };
    
    union Storage {
        int i;
        float f;
        Storage() : i(0) {}
    };
    
    Mode mode;
    ComplexStructure() : mode(Mode::READ_ONLY) {}
};

// ============================================================================
// TEMPLATE STRUCT WITH NESTED TYPES
// ============================================================================

template<typename T>
struct TemplateStructure {
    enum class State { INITIALIZED, PROCESSING };
    
    struct Metadata {
        int version;
        Metadata() : version(1) {}
    };
    
    template<typename U>
    struct Pair {
        T first;
        U second;
        Pair() : first(T()), second(U()) {}
    };
    
    T data;
    State state;
    TemplateStructure() : state(State::INITIALIZED) {}
};

// ============================================================================
// NAMESPACE WITH NESTED STRUCTURES
// ============================================================================

namespace Network {
    class Connection;
    enum class Protocol;
    
    class Connection {
    public:
        struct ConnectionInfo;
        enum class ConnectionState;
        template<typename T> class Buffer;
        
        Connection();
        void open();
        
    private:
        ConnectionState state;
    };
    
    struct Connection::ConnectionInfo {
        std::string host;
        int port;
        ConnectionInfo() : port(0) {}
    };
    
    enum class Connection::ConnectionState { CLOSED, OPEN };
    
    template<typename T>
    class Connection::Buffer {
    private:
        std::vector<T> data;
    public:
        Buffer() {}
        void push(const T& item) { data.push_back(item); }
    };
    
    enum class Protocol { TCP, UDP };
    
    template<typename T>
    class Message {
    public:
        struct Header {
            int messageId;
            Header() : messageId(0) {}
        };
        
        enum class Priority { LOW, HIGH };
        
        Message() : priority(Priority::LOW) {}
        
    private:
        Header header;
        T payload;
        Priority priority;
    };
}

// ============================================================================
// CLASS WITH OUT-OF-CLASS METHOD DEFINITIONS
// ============================================================================

class Engine {
public:
    struct EngineSpecs;
    enum class EngineType;
    
    Engine();
    void start();
    int getRPM() const;
    template<typename T> T calculateEfficiency(T fuel, T distance);
    static EngineType getDefaultType();
    
private:
    EngineType type;
    int rpm;
    void updateTemperature();
};

struct Engine::EngineSpecs {
    double displacement;
    EngineSpecs() : displacement(2.0) {}
};

enum class Engine::EngineType { INLINE, V_TYPE, ELECTRIC };

// ============================================================================
// NAMESPACE WITH DATA STRUCTURES
// ============================================================================

namespace DataStructures {
    template<typename T>
    class Stack {
    private:
        std::vector<T> elements;
    public:
        void push(const T& element) { elements.push_back(element); }
        void pop() { if (!elements.empty()) elements.pop_back(); }
        bool empty() const { return elements.empty(); }
    };
    
    template<typename T>
    struct TreeNode {
        T data;
        std::shared_ptr<TreeNode<T>> left, right;
        TreeNode(const T& value) : data(value), left(nullptr), right(nullptr) {}
    };
}

// ============================================================================
// INHERITANCE EXAMPLE
// ============================================================================

class Shape {
protected:
    Color color;
public:
    Shape(Color c = Color::RED) : color(c) {}
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual void draw() const = 0;
};

class Circle : public Shape {
private:
    double radius;
public:
    Circle(double r) : Shape(), radius(r) {}
    double area() const override { return 3.14159 * radius * radius; }
    void draw() const override { std::cout << "Circle\n"; }
};

// ============================================================================
// TEMPLATE CLASSES
// ============================================================================

template<typename T1, typename T2>
class Pair {
private:
    T1 first;
    T2 second;
public:
    Pair() : first(T1()), second(T2()) {}
    T1 getFirst() const { return first; }
    bool operator==(const Pair<T1, T2>& other) const {
        return first == other.first && second == other.second;
    }
};

template<typename T>
class Container {
private:
    std::vector<T> items;
public:
    void add(const T& item) { items.push_back(item); }
    size_t size() const { return items.size(); }
    typename std::vector<T>::iterator begin() { return items.begin(); }
};

// ============================================================================
// ADVANCED TEMPLATES
// ============================================================================

// Variadic template
template<typename T>
T sum(T value) { return value; }

template<typename T, typename... Args>
T sum(T first, Args... args) { return first + sum(args...); }

// Template specialization
template<typename T>
class TypeInfo {
public:
    static std::string name() { return "Unknown"; }
};

template<>
class TypeInfo<int> {
public:
    static std::string name() { return "Integer"; }
};

// SFINAE
template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
isEven(T value) { return value % 2 == 0; }

// Template metaprogramming
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

// Type traits
template<typename T>
struct is_pointer { static constexpr bool value = false; };

template<typename T>
struct is_pointer<T*> { static constexpr bool value = true; };

template<typename T>
struct remove_const { using type = T; };

template<typename T>
struct remove_const<const T> { using type = T; };

// Variadic type operations
template<typename... Args>
struct count_args { static constexpr size_t value = sizeof...(Args); };

template<size_t Index, typename... Types>
struct type_at;

template<typename First, typename... Rest>
struct type_at<0, First, Rest...> { using type = First; };

template<size_t Index, typename First, typename... Rest>
struct type_at<Index, First, Rest...> {
    using type = typename type_at<Index - 1, Rest...>::type;
};

// Tuple implementation
template<typename... Types>
class Tuple;

template<>
class Tuple<> {
public:
    Tuple() {}
};

template<typename Head, typename... Tail>
class Tuple<Head, Tail...> : private Tuple<Tail...> {
private:
    Head head;
public:
    Tuple() : head(), Tuple<Tail...>() {}
    Head& getHead() { return head; }
};

// Template with fixed size
template<typename T, size_t N>
class Array {
private:
    T data[N];
public:
    T& operator[](size_t index) { return data[index]; }
    constexpr size_t size() const { return N; }
};

// Policy-based design
template<typename T, template<typename> class CreationPolicy>
class SmartPtr : public CreationPolicy<T> {
private:
    T* ptr;
public:
    SmartPtr() : ptr(CreationPolicy<T>::create()) {}
    ~SmartPtr() { CreationPolicy<T>::destroy(ptr); }
    T& operator*() { return *ptr; }
};

template<typename T>
class NewCreator {
public:
    static T* create() { return new T(); }
    static void destroy(T* p) { delete p; }
};

// ============================================================================
// COMPLEX NESTED CLASS
// ============================================================================

class ComplexClass {
public:
    struct NestedStruct;
    class NestedClass;
    enum class NestedEnum;
    union NestedUnion;
    template<typename T> struct NestedTemplateStruct;
    
    ComplexClass();
    void simpleMethod();
    template<typename T> T templateMethod(T value);
    
private:
    int data;
};

struct ComplexClass::NestedStruct {
    int x;
    enum class StructEnum { OPTION_A, OPTION_B };
    NestedStruct() : x(0) {}
};

class ComplexClass::NestedClass {
public:
    struct ClassStruct {
        double value;
        ClassStruct() : value(0.0) {}
    };
    NestedClass() {}
};

enum class ComplexClass::NestedEnum { VALUE_1, VALUE_2 };

union ComplexClass::NestedUnion {
    int intValue;
    float floatValue;
    NestedUnion() : intValue(0) {}
};

template<typename T>
struct ComplexClass::NestedTemplateStruct {
    T data;
    enum class TemplateStructEnum { TYPE_A, TYPE_B };
    NestedTemplateStruct() : data(T()) {}
};

// ============================================================================
// MEGA COMPLEX TEMPLATE
// ============================================================================

template<typename T, typename U = int>
class MegaComplexTemplate {
public:
    struct Data;
    enum class Mode;
    template<typename V> struct Container;
    
    MegaComplexTemplate();
    template<typename V> void process(const V& value);
    
private:
    T primaryData;
    U secondaryData;
};

template<typename T, typename U>
struct MegaComplexTemplate<T, U>::Data {
    T value;
    enum class DataState { VALID, INVALID };
    struct DataInfo {
        int version;
        DataInfo() : version(1) {}
    };
    DataState state;
    Data() : value(T()), state(DataState::VALID) {}
};

template<typename T, typename U>
enum class MegaComplexTemplate<T, U>::Mode { SYNCHRONOUS, ASYNCHRONOUS };

template<typename T, typename U>
template<typename V>
struct MegaComplexTemplate<T, U>::Container {
    V data;
    T primaryKey;
    Container() : data(V()), primaryKey(T()) {}
};

// ============================================================================
// NAMESPACE WITH DEEP NESTING
// ============================================================================

namespace ComplexNamespace {
    class OuterClass {
    public:
        struct InnerStruct {
            enum class InnerEnum { A, B };
            template<typename T>
            struct InnerTemplateStruct {
                T value;
                InnerTemplateStruct() : value(T()) {}
            };
            int data;
            InnerStruct() : data(0) {}
        };
        OuterClass() {}
    };
    
    template<typename T>
    class OuterTemplate {
    public:
        struct TemplateStruct {
            T value;
            enum class StructEnum { ALPHA, BETA };
            TemplateStruct() : value(T()) {}
        };
        
        template<typename U>
        class InnerTemplate {
        public:
            struct InnerStruct {
                T outerValue;
                U innerValue;
                InnerStruct() : outerValue(T()), innerValue(U()) {}
            };
            InnerTemplate() {}
        };
        OuterTemplate() {}
    };
}

// ============================================================================
// OUT-OF-CLASS METHOD DEFINITIONS
// ============================================================================

inline Vehicle::Vehicle() : make(""), speed(0.0) { vehicleCount++; }
inline Vehicle::~Vehicle() { vehicleCount--; }
inline void Vehicle::start() { std::cout << "Vehicle started\n"; }
inline double Vehicle::getSpeed() const { return speed; }
template<typename T>
inline void Vehicle::addComponent(const T& component) { std::cout << "Component added\n"; }
inline int Vehicle::getVehicleCount() { return vehicleCount; }

template<typename T>
inline Node<T>::Node() : value(T()), type(NodeType::LEAF) {}

template<typename T>
inline T Node<T>::getValue() const { return value; }

template<typename T, typename U>
inline Graph<T, U>::Graph() {}

template<typename T, typename U>
inline void Graph<T, U>::addNode(const T& nodeData) {
    GraphNode node;
    node.data = nodeData;
    nodes.push_back(node);
}

inline Network::Connection::Connection() : state(ConnectionState::CLOSED) {}
inline void Network::Connection::open() { state = ConnectionState::OPEN; int a;}

inline Engine::Engine() : type(EngineType::INLINE), rpm(0) {}
inline void Engine::start() { rpm = 1000; }
inline int Engine::getRPM() const { return rpm; }
template<typename T>
inline T Engine::calculateEfficiency(T fuel, T distance) {
    using type = T;
    return fuel == 0 ? 0 : distance / fuel;
}
inline Engine::EngineType Engine::getDefaultType() { return EngineType::INLINE; }
inline void Engine::updateTemperature() {}

inline ComplexClass::ComplexClass() : data(0) {}
inline void ComplexClass::simpleMethod() {}
template<typename T>
inline T ComplexClass::templateMethod(T value) { return value; }

template<typename T, typename U>
inline MegaComplexTemplate<T, U>::MegaComplexTemplate() 
    : primaryData(T()), secondaryData(U()) {}

template<typename T, typename U>
template<typename V>
inline void MegaComplexTemplate<T, U>::process(const V& value) {
    std::cout << "Processing\n";
}

// Static member initialization
inline int Vehicle::vehicleCount = 0;

#endif // LIB_H
