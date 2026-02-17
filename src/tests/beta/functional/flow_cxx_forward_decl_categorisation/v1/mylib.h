// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADVANCED_LIBRARY_HPP
#define ADVANCED_LIBRARY_HPP

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <type_traits>
#include <tuple>
#include <queue>
#include <thread>
#include <variant>
#include <optional>
#include <future>
#include <mutex>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <algorithm>

// Forward declarations
namespace core {
    template<typename T>
    class SmartContainer;
    
    template<typename Key, typename Value>
    class CacheManager;
    
    class ResourceManager;
    class ThreadPool;
}

namespace utils {
    template<typename... Args>
    class TypeList;
    
    template<typename T>
    struct TypeTraits;
    
    class Logger;
}

namespace algorithms {
    template<typename Iterator, typename Predicate>
    class FilterIterator;
    
    template<typename T, template<typename> class Container>
    class SortedContainer;
}

namespace patterns {
    template<typename T>
    class Singleton;
    
    template<typename Event>
    class Observer;
    
    template<typename Event>
    class Subject;
}

// Type trait helpers (C++17 compatible)
template<typename T, typename = void>
struct is_serializable : std::false_type {};

template<typename T>
struct is_serializable<T, std::void_t<
    decltype(std::declval<T>().serialize()),
    decltype(T::deserialize(std::string{}))
>> : std::true_type {};

template<typename T>
constexpr bool is_serializable_v = is_serializable<T>::value;

template<typename T, typename = void>
struct is_comparable : std::false_type {};

template<typename T>
struct is_comparable<T, std::void_t<
    decltype(std::declval<T>() < std::declval<T>()),
    decltype(std::declval<T>() == std::declval<T>())
>> : std::true_type {};

template<typename T>
constexpr bool is_comparable_v = is_comparable<T>::value;

template<typename T, typename = void>
struct is_hashable : std::false_type {};

template<typename T>
struct is_hashable<T, std::void_t<
    decltype(std::hash<T>{}(std::declval<T>()))
>> : std::true_type {};

template<typename T>
constexpr bool is_hashable_v = is_hashable<T>::value;

// Core namespace implementation
namespace core {
    
    // SFINAE helper templates
    template<typename T, typename = void>
    struct has_size : std::false_type {};
    
    template<typename T>
    struct has_size<T, decltype(std::declval<T>().size(), void())> : std::true_type {};
    
    template<typename T>
    constexpr bool has_size_v = has_size<T>::value;
    
    // Variadic template for type manipulation
    template<typename... Types>
    struct TypePack {
        static constexpr std::size_t size = sizeof...(Types);
        
        template<std::size_t N>
        using type_at = std::tuple_element_t<N, std::tuple<Types...>>;
    };
     
    // Cache manager with LRU eviction policy
    template<typename Key, typename Value>
    class CacheManager {
    private:
        struct CacheNode {
            Key key;
            Value value;
            std::chrono::steady_clock::time_point timestamp;
            std::size_t access_count{0};
            
            CacheNode(Key k, Value v) 
                : key(std::move(k)), value(std::move(v)), 
                  timestamp(std::chrono::steady_clock::now()) {}
        };
        
        std::unordered_map<Key, std::unique_ptr<CacheNode>> cache_;
        std::size_t max_size_;
        mutable std::mutex mutex_;
        
        void evict_lru();
        
    public:
        explicit CacheManager(std::size_t max_size = 1000);
        
        void put(const Key& key, const Value& value);
        void put(Key&& key, Value&& value);
        
        bool get(const Key& key, Value& value);
        
        bool contains(const Key& key) const;
        void remove(const Key& key);
        void clear();
        
        std::size_t size() const;
        std::size_t max_size() const { return max_size_; }
        
        // Statistics
        struct Statistics {
            std::size_t hits{0};
            std::size_t misses{0};
            std::size_t evictions{0};
            double hit_ratio() const { 
                return hits + misses > 0 ? static_cast<double>(hits) / (hits + misses) : 0.0; 
            }
        };
        
        Statistics get_statistics() const;
    };
    
    // Resource manager with RAII
    class ResourceManager {
    private:
        struct Resource {
            std::string name;
            std::function<void()> deleter;
            std::chrono::steady_clock::time_point created_at;
            
            Resource(std::string n, std::function<void()> d)
                : name(std::move(n)), deleter(std::move(d)),
                  created_at(std::chrono::steady_clock::now()) {}
        };
        
        std::vector<std::unique_ptr<Resource>> resources_;
        mutable std::mutex mutex_;
        
    public:
        template<typename T, typename... Args>
        std::shared_ptr<T> create_resource(const std::string& name, Args&&... args);
        
        void release_resource(const std::string& name);
        void release_all();
        
        std::size_t resource_count() const;
        std::vector<std::string> get_resource_names() const;
        
        ~ResourceManager();
    };
    
    // Thread pool implementation
    class ThreadPool {
    private:
        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;
        std::mutex queue_mutex_;
        std::condition_variable condition_;
        std::atomic<bool> stop_{false};
        
    public:
        explicit ThreadPool(std::size_t num_threads = std::thread::hardware_concurrency());
        
        template<typename F, typename... Args>
        auto enqueue(F&& f, Args&&... args) 
            -> std::future<typename std::result_of<F(Args...)>::type>;
        
        std::size_t size() const { return workers_.size(); }
        
        ~ThreadPool();
    };
}

// Utils namespace implementation
namespace utils {
    
    // Compile-time type list manipulation
    template<typename... Args>
    class TypeList {
    public:
        static constexpr std::size_t size = sizeof...(Args);
        
        template<std::size_t N>
        using at = std::tuple_element_t<N, std::tuple<Args...>>;
        
        template<typename T>
        struct contains_helper;
        
        template<typename T>
        static constexpr bool contains = contains_helper<T>::value;
        
        template<template<typename> class Predicate>
        struct count_if_helper;
        
        template<template<typename> class Predicate>
        static constexpr std::size_t count_if = count_if_helper<Predicate>::value;
    };
    
    // Advanced type traits
    template<typename T>
    struct TypeTraits {
        static constexpr bool is_pointer = std::is_pointer<T>::value;
        static constexpr bool is_reference = std::is_reference<T>::value;
        static constexpr bool is_const = std::is_const<typename std::remove_reference<T>::type>::value;
        static constexpr bool is_volatile = std::is_volatile<typename std::remove_reference<T>::type>::value;
        
        using bare_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        using const_type = typename std::add_const<T>::type;
        using reference_type = typename std::add_lvalue_reference<T>::type;
        using pointer_type = typename std::add_pointer<T>::type;
        
        template<typename U>
        static constexpr bool is_same_bare = std::is_same<bare_type, typename TypeTraits<U>::bare_type>::value;
    };
    
    // Logging utility with different levels
    enum class LogLevel {
        DEBUG, INFO, WARNING, ERROR, CRITICAL
    };
    
    class Logger {
    private:
        LogLevel min_level_;
        std::mutex mutex_;
        std::vector<std::function<void(LogLevel, const std::string&)>> handlers_;
        
    public:
        explicit Logger(LogLevel min_level = LogLevel::INFO);
        
        void add_handler(std::function<void(LogLevel, const std::string&)> handler);
        
        template<typename... Args>
        void log(LogLevel level, const std::string& format, Args&&... args);
        
        template<typename... Args>
        void debug(const std::string& format, Args&&... args) {
            log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
        }
        
        template<typename... Args>
        void info(const std::string& format, Args&&... args) {
            log(LogLevel::INFO, format, std::forward<Args>(args)...);
        }
        
        template<typename... Args>
        void warning(const std::string& format, Args&&... args) {
            log(LogLevel::WARNING, format, std::forward<Args>(args)...);
        }
        
        template<typename... Args>
        void error(const std::string& format, Args&&... args) {
            log(LogLevel::ERROR, format, std::forward<Args>(args)...);
        }
        
        template<typename... Args>
        void critical(const std::string& format, Args&&... args) {
            log(LogLevel::CRITICAL, format, std::forward<Args>(args)...);
        }
        
        void set_min_level(LogLevel level) { min_level_ = level; }
        LogLevel get_min_level() const { return min_level_; }
    };
}

// Algorithms namespace implementation
namespace algorithms {
    
    // Filter iterator for lazy evaluation
    template<typename Iterator, typename Predicate>
    class FilterIterator {
    private:
        Iterator current_;
        Iterator end_;
        Predicate predicate_;
        
        void advance_to_valid() {
            while (current_ != end_ && !predicate_(*current_)) {
                ++current_;
            }
        }
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        using pointer = typename std::iterator_traits<Iterator>::pointer;
        using reference = typename std::iterator_traits<Iterator>::reference;
        
        FilterIterator(Iterator current, Iterator end, Predicate pred)
            : current_(current), end_(end), predicate_(pred) {
            advance_to_valid();
        }
        
        reference operator*() const { return *current_; }
        pointer operator->() const { return &(*current_); }
        
        FilterIterator& operator++() {
            ++current_;
            advance_to_valid();
            return *this;
        }
        
        FilterIterator operator++(int) {
            auto temp = *this;
            ++(*this);
            return temp;
        }
        
        bool operator==(const FilterIterator& other) const {
            return current_ == other.current_;
        }
        
        bool operator!=(const FilterIterator& other) const {
            return !(*this == other);
        }
    };
    
    // Sorted container maintaining order
    template<typename T, template<typename> class Container = std::vector>
    class SortedContainer {
    private:
        Container<T> data_;
        std::function<bool(const T&, const T&)> comparator_;
        
    public:
        using value_type = T;
        using iterator = typename Container<T>::iterator;
        using const_iterator = typename Container<T>::const_iterator;
        
        explicit SortedContainer(std::function<bool(const T&, const T&)> comp = std::less<T>{})
            : comparator_(std::move(comp)) {}
        
        void insert(const T& value);
        void insert(T&& value);
        
        template<typename... Args>
        void emplace(Args&&... args);
        
        bool remove(const T& value);
        
        template<typename Predicate>
        std::size_t remove_if(Predicate pred);
        
        const_iterator find(const T& value) const;
        const_iterator lower_bound(const T& value) const;
        const_iterator upper_bound(const T& value) const;
        
        std::size_t size() const { return data_.size(); }
        bool empty() const { return data_.empty(); }
        
        const_iterator begin() const { return data_.begin(); }
        const_iterator end() const { return data_.end(); }
        const_iterator cbegin() const { return data_.cbegin(); }
        const_iterator cend() const { return data_.cend(); }
    };
}

// Design patterns namespace
namespace patterns {
    
    // Thread-safe singleton
    template<typename T>
    class Singleton {
    private:
        static std::once_flag initialized_;
        static std::unique_ptr<T> instance_;
        
        Singleton() = default;
        
    public:
        static T& get_instance() {
            std::call_once(initialized_, []() {
                instance_ = std::make_unique<T>();
            });
            return *instance_;
        }
        
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(Singleton&&) = delete;
    };
    
    template<typename T>
    std::once_flag Singleton<T>::initialized_;
    
    template<typename T>
    std::unique_ptr<T> Singleton<T>::instance_;
    
    // Observer pattern implementation
    template<typename Event>
    class Observer {
    public:
        virtual ~Observer() = default;
        virtual void on_event(const Event& event) = 0;
    };
    
    template<typename Event>
    class Subject {
    private:
        std::vector<std::weak_ptr<Observer<Event>>> observers_;
        mutable std::mutex mutex_;
        
        void cleanup_expired_observers() const;
        
    public:
        void attach(std::shared_ptr<Observer<Event>> observer);
        void detach(std::shared_ptr<Observer<Event>> observer);
        
        void notify(const Event& event);
        
        std::size_t observer_count() const;
    };
}

// Global utility functions
template<typename Container, typename Predicate>
auto make_filter_range(Container& container, Predicate pred) {
    using Iterator = decltype(container.begin());
    return std::make_pair(
        algorithms::FilterIterator<Iterator, Predicate>(container.begin(), container.end(), pred),
        algorithms::FilterIterator<Iterator, Predicate>(container.end(), container.end(), pred)
    );
}

template<typename T>
const char* make_type_name() {
    #ifdef __GNUC__
        return __PRETTY_FUNCTION__;
    #elif defined(_MSC_VER)
        return __FUNCSIG__;
    #else
        return "unknown";
    #endif
}

// Compile-time string hashing
constexpr std::size_t hash_string(const char* str, std::size_t hash = 5381) {
    return *str ? hash_string(str + 1, hash * 33 + static_cast<unsigned char>(*str)) : hash;
}

// User-defined literals
namespace literals {
    constexpr std::size_t operator""_hash(const char* str, std::size_t) {
        return hash_string(str);
    }
}

// External template variable declarations for Singleton static members
template std::once_flag patterns::Singleton<int>::initialized_;
template std::unique_ptr<int> patterns::Singleton<int>::instance_;

#endif // ADVANCED_LIBRARY_HPP