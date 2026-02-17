cmake_minimum_required(VERSION 3.14)
project(armor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
    OUTPUT_VARIABLE DETECTED_GCC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CLANG_EXTRA_FLAGS
    "-std=c++17 -xc++ -isystem /usr/include/c++/11 -isystem /usr/include -isystem /usr/local/include -isystem /usr/include/x86_64-linux-gnu/c++/11 -I/usr/lib/llvm-14/include -I/usr/lib/clang/14/include"
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
message(STATUS "Detected compiler version: ${DETECTED_GCC_VERSION}")

llvm_map_components_to_libnames(LLVM_LIBS
  support
  core
  option
)

add_definitions(-DTOOL_VERSION="0.6.3")

add_subdirectory(src/common)
add_subdirectory(src/alpha)
add_subdirectory(src/beta)
add_subdirectory(src/armor)

add_subdirectory(src/tests/alpha/src)
add_subdirectory(src/tests/beta/src)
add_subdirectory(src/tests/armor/src)
add_subdirectory(src/tests/common)


add_custom_target(build_all_executables ALL
        DEPENDS armor alpha beta
        COMMENT "Building all executables"
)
