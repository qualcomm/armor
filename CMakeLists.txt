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

message(STATUS "Detected compiler version: ${DETECTED_GCC_VERSION}")

add_subdirectory(src/common)
add_subdirectory(src/alpha)
add_subdirectory(src/beta)
add_subdirectory(src/armor)
add_subdirectory(src/tests/alpha/src)
add_subdirectory(src/tests/beta/src)


add_custom_target(build_all_executables ALL
        DEPENDS armor alpha beta
        COMMENT "Building all executables"
)
