cmake_minimum_required(VERSION 3.14)
project(armor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CLANG_EXTRA_FLAGS
    "-xc++ -isystem /usr/include/c++/11 -isystem /usr/include -isystem /usr/local/include -isystem /usr/include/x86_64-linux-gnu/c++/11 -I/usr/lib/llvm-14/include -I/usr/lib/clang/14/include"
)

string(REPLACE "\"" "\\\"" CLANG_EXTRA_FLAGS_ESCAPED "${CLANG_EXTRA_FLAGS}")

add_compile_definitions(CLANG_FLAGS="${CLANG_EXTRA_FLAGS_ESCAPED}")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Clang include dirs: ${CLANG_INCLUDE_DIRS}")
message(STATUS "Detected compiler version: ${DETECTED_GCC_VERSION}")
message(STATUS "Injected Clang flags: ${CLANG_EXTRA_FLAGS}")

file(GLOB_RECURSE ALPHA_SOURCES "src/*.cpp")

add_library(alpha_lib STATIC
  ${ALPHA_SOURCES}
)

target_include_directories(alpha_lib PUBLIC
  ${CMAKE_SOURCE_DIR}/src/alpha/include
  ${CMAKE_SOURCE_DIR}/src/common/include
)

llvm_map_components_to_libnames(LLVM_LIBS
  support
  core
  option
)

target_link_libraries(alpha_lib
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  common_lib
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)
