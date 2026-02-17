// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
struct alpha{
    int a;
    alpha(){
        a = 0;
    }
    alpha(int x, int y){


        // new comment
        a = x;
        a = y;
    }
};

alpha a;

alpha b(
    
    /*
    new comments
    */
    
    1,2);

alpha c;

struct beta{



    // new comment
    int a = 2;
    int b;
    alpha c;
} beta_var;

// Complex typedef with function pointer and nested structures
typedef struct {
    int (*operation)(int, int);
    union {
        struct {
            double real;
            double imag;
        } complex;
        long long integer;
    } data;
    enum { ADD, SUB, MUL, DIV } op_type;
} ComplexCalculator;

// 3-level nested structures with enums and unions
struct OuterLevel {
    enum Status { ACTIVE = 1, INACTIVE = 0, PENDING = 2 } status;
    
    struct MiddleLevel {
        union DataType {
            int int_val;
            float float_val;
            char char_val;
        } data;
        
        struct InnerLevel {
            enum Priority { LOW, MEDIUM, HIGH } priority;
            union {
                struct {
                    int x, y, z;
                } coordinates;
                char buffer[12];
            } payload;
            bool is_valid = true;
            int counter = 0;
            double weight = 1.0;
            char tag[8] = "inner";
        } inner;
        
        int middle_id = 100;
        float scale = 2.5f;
        char name[16] = "default";
        bool active = true;
    } middle;
    
    int outer_id = 1;
    bool enabled = true;
    double version = 1.2;
    char description[32] = "outer_level";
} nested_instance = { 
    .status = OuterLevel::ACTIVE,
    .middle = {
        .data = { .int_val = 42 },
        .inner = {
            .priority = OuterLevel::MiddleLevel::InnerLevel::HIGH,
            .payload = { .coordinates = {10, 20, 30} },
            .counter = 5,
            .weight = 3.14,
  
            .tag = "test"
        },
        .middle_id = 200,
        .scale = 1.5f,
        .name = "middle_node",
        .active = false
    },
    .outer_id = 999,
    .enabled = false,
    .version = 2.1,
    .description = "test_instance"
};

// Function with definition
int calculate_sum(int a, int b, int c = 5) {
    return a + b + c;
}

// Function declaration with default parameters
double process_data(const ComplexCalculator* calc, 
                   double input = 1.0, 
                   bool normalize = true, 
                   int iterations = 10);
struct OuterLevel nested_array[3] = {
    {
        OuterLevel::ACTIVE,
        {
            { .float_val = 1.1f },
            {
                OuterLevel::MiddleLevel::InnerLevel::LOW,
                { .buffer = "test1" },
                false,
                1,
                0.5
            },
            101,
            1.0f,
            "first"
        },
        1001,
        true,
        1.0
    },
    {
        .status = OuterLevel::PENDING,
        .middle = {
            .data = { .char_val = 'X' },
            .inner = {
                .priority = OuterLevel::MiddleLevel::InnerLevel::MEDIUM,
                .payload = { .coordinates = {100, 200, 300} }
            }
        }
    },
    {}
};

// Struct with complex member initializations
struct ConfigData {
    int values[5] = {1, 2, 3, 4, 5};
    double matrix[2][2] = {{1.0, 2.0}, {3.0, 4.0}};
    char text[32] = "initialized";
    bool flags[4] = {true, false, true, false};
    
    struct Point {
        int x = 10;
        int y = 20;
        char label[8] = "origin";
    } points[2] = {
        {100, 200, "point1"},
        {300, 400, "point2"}
    };
} config_instance;

// Union with different initialization patterns
union MultiType {
    int as_int;
    float as_float;
    char as_chars[4];
    struct {
        short low;
        short high;
    } as_shorts;
};

MultiType multi1 = { .as_int = 42 };
MultiType multi2 = { .as_float = 3.14f };
MultiType multi3 = { .as_chars = {'A', 'B', 'C', '\0'} };
MultiType multi4 = { .as_shorts = {100, 200} };

// Enhanced nested structure with more initializers
struct OuterLevel enhanced_nested = {
    .status = OuterLevel::INACTIVE,
    .middle = {
        .data = { .int_val = 999 },
        // new comments
        .inner = {
            .priority = OuterLevel::MiddleLevel::InnerLevel::HIGH,
            .payload = { .buffer = "enhanced" },
            .is_valid = true
        },
        .middle_id = 500
    },
    .outer_id = 2000
};

// Array of structures with mixed initialization styles
struct beta beta_array[4] = {
    {10, 20, alpha()},
    {.a = 30, 
        
        /*
        new comments
        */
        .b = 40, .c = alpha(5, 6)},
    {50, 60},
    {}
};

// Complex array initialization
int int_matrix[3][4] = {
    {1, 2, 3, 4},
    {5
        /*


        new comments
        */, 6, 7, 8},
    // new comments
    {9, 10, 11, 12}
};

// String array with initializers
const char* string_array[5] = {
    "first", // new comments
    "second", // new comments
    "third", // new comments
    "fourth", // new comments

    // new comments
    nullptr // new comments
};

