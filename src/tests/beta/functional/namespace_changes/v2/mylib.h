// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <iostream>

// Top-level named namespace
namespace outer {

// First level named namespace
namespace level1 {

    // Class in named namespace
    class OuterClass {
    public:
        void method() const { /* implementation */ }
    };

    // Anonymous namespace inside named namespace
    namespace {
        // Function in anonymous namespace
        void helperFunction() {
            // Implementation
        }

        // Second level named namespace inside anonymous namespace
        namespace inner_level2 {
            // Struct in named namespace inside anonymous namespace
            struct InnerStruct {
                int value;
                double ratio;
            };

            // Enum in deeply nested namespace
            enum class Priority {
                LOW,
                MEDIUM,
                HIGH
            };

            // Another anonymous namespace (third level)
            namespace {
                // Global variable in deeply nested anonymous namespace
                InnerStruct defaultStruct{42, 3.14};

                // Function in deeply nested anonymous namespace
                Priority calculatePriority(int value) {
                    return value > 100 ? Priority::HIGH : 
                           value > 50 ? Priority::MEDIUM : 
                           Priority::LOW;
                }

                // Fourth level named namespace inside anonymous namespace
                namespace detail {
                    // Template class in deeply nested namespace
                    template<typename T>
                    class Container {
                    public:
                        void add(const T& item) {
                            items.push_back(item);
                        }
                    private:
                        std::vector<T> items;
                    };

                    // Function with function pointer parameter
                    void process(void (*callback)(void*), void* data) {
                        callback(data);
                    }

                    // Anonymous namespace (fifth level)
                    namespace {
                        // Static variable in deepest anonymous namespace
                        static int counter = 0;

                        // Union in deepest anonymous namespace
                        union DataPacket {
                            struct {
                                uint8_t type;
                                uint16_t length;
                                uint8_t data[1024];
                            } structured;
                            uint8_t raw[1027];
                        };

                        // Class with virtual methods
                        class DeepProcessor {
                        public:
                            virtual ~DeepProcessor() = default;
                            virtual void process() = 0;
                            
                            // Static method returning function pointer
                            static auto getDefaultHandler() -> void (*)(const std::string&) {
                                return [](const std::string& msg) { /* handle */ };
                            }
                        };
                    } // deepest anonymous namespace
                } // namespace detail
            } // third level anonymous namespace
        } // namespace inner_level2

        // Another named namespace at second level
        namespace alternative {
            // Class with friend class
            class Manager {
            public:
                Manager() = default;
                
                // Method with const and volatile qualifiers
                int getStatus() const volatile { return status; }
                
            private:
                int status = 0;
                
                // Friend class in same namespace
                friend class Controller;
            };
            
            // Friend class implementation
            class Controller {
            public:
                void reset(Manager& manager) {
                    manager.status = 0;  // Can access private member
                }
            };
                
            // Anonymous namespace inside class friend context
            namespace {
                // Function in anonymous namespace
                void initializeSystem() {
                    // Implementation
                }
                
                // Nested named namespace (third level)
                namespace impl {
                    // Template with specialization
                    template<typename T>
                    struct TypeTraits {
                        static constexpr bool isPointer = false;
                    };
                    
                    template<typename T>
                    struct TypeTraits<T*> {
                        static constexpr bool isPointer = true;
                        using BaseType = T;
                    };
                    
                    // Anonymous namespace (fourth level)
                    namespace {
                        // Class with deleted and defaulted methods
                        class ResourceHandler {
                        public:
                            ResourceHandler() = default;
                            ~ResourceHandler() = default;
                            
                            ResourceHandler(const ResourceHandler&) = delete;
                            ResourceHandler& operator=(const ResourceHandler&) = delete;
                            
                            ResourceHandler(ResourceHandler&&, int);
                            ResourceHandler& operator=(ResourceHandler&&) = default;

                            ResourceHandler& operator+(ResourceHandler&&);
                            
                            // Method with override and final
                            void cleanup() { /* implementation */ }
                        };
                    } // fourth level anonymous namespace
                } // namespace impl
            } // anonymous namespace inside Controller context


            
        } // namespace alternative
    } // first level anonymous namespace
} // namespace level1

// Another named namespace at first level
namespace utilities {
    // Function template
    template<typename T>
    T clamp(T value, T min, T max) {
        return value < min ? min : (value > max ? max : value);
    }
    
    // Anonymous namespace
    namespace {
        // Global constants
        constexpr double PI = 3.14159265358979323846;
        constexpr double E = 2;
        
        // Named namespace inside anonymous namespace
        namespace math {
            // Function using outer scope constants
            double calculateCircleArea(double radius) {
                return PI * radius * radius;
            }
            
            // Class with explicit constructor
            class Vector {
            public:
                explicit Vector(int dimensions) : size(dimensions) {}
                
                // Method with const qualifier
                void getDimensions() const { return; }
                
            private:
                int size;
            };
            
            // Anonymous namespace (third level)
            namespace new_namespace{
                // Struct with nested enum
                struct Algorithm {
                    enum Type {
                        FAST,
                        ACCURATE,
                        BALANCED
                    };
                    
                    Type type;
                    bool parallel;
                };

                class NewClass{
                    int a;
                };
                
                // Global variable
                // Algorithm defaultAlgorithm{Algorithm::BALANCED, true};
                
                // Named namespace (fourth level)
                namespace advanced {
                    // Class with virtual inheritance
                    class BaseProcessor {
                    public:
                        virtual ~BaseProcessor() = default;
                        virtual void execute() = 0;
                    };
                    
                    class FastProcessor : public virtual BaseProcessor {
                    public:
                        void execute() override { /* implementation */ }
                    };
                    
                    class AccurateProcessor : public virtual BaseProcessor {
                    public:
                        void execute() override { /* implementation */ }
                    };
                    
                    // Multiple inheritance
                    class HybridProcessor : public FastProcessor, public AccurateProcessor {
                    public:
                        void execute() override { /* implementation */ }
                    };
                    
                    // Anonymous namespace (fifth level)
                    namespace {
                        // Function that returns lambda
                        auto createProcessor(Algorithm::Type type) {    
                            return [](int x) { return x * 2; };
                        }
                    } // fifth level anonymous namespace
                } // namespace advanced
            } // third level anonymous namespace
        } // namespace math
    } // anonymous namespace in utilities
} // namespace utilities

}

outer::level1::OuterClass obj;

void testNamespaces() {
    // Create objects from outer::level1 namespace
    outer::level1::OuterClass obj;
    // Create objects from utilities namespace
    auto clampedValue = outer::utilities::clamp(42, 0, 100);
}
