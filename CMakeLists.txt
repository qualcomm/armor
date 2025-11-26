cmake_minimum_required(VERSION 3.14)
project(armor)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: Set Clang explicitly
# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)

# Detect compiler version
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
    OUTPUT_VARIABLE DETECTED_GCC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# if(NOT DETECTED_GCC_VERSION MATCHES "^11")
#     message(FATAL_ERROR "This project requires GCC 11. Detected version: ${DETECTED_GCC_VERSION}")
# endif()

# Construct Clang flags
set(CLANG_EXTRA_FLAGS
    "-xc++ -include stdint.h -isystem /usr/include/c++/11 -isystem /usr/include -isystem /usr/local/include -isystem /usr/include/x86_64-linux-gnu/c++/11 -I/usr/lib/llvm-14/include -I/usr/lib/clang/14/include"
)

# Escape quotes for macro definition
string(REPLACE "\"" "\\\"" CLANG_EXTRA_FLAGS_ESCAPED "${CLANG_EXTRA_FLAGS}")

# Pass flags as a macro to C++
add_compile_definitions(CLANG_FLAGS="${CLANG_EXTRA_FLAGS_ESCAPED}")

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# Include LLVM and Clang headers
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})



# Optional: Print debug info
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Clang include dirs: ${CLANG_INCLUDE_DIRS}")
message(STATUS "Detected compiler version: ${DETECTED_GCC_VERSION}")
message(STATUS "Injected Clang flags: ${CLANG_EXTRA_FLAGS}")


# Fetch dependencies
include(FetchContent)

# nlohmann/json v3.12.0
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)

# CLI11
FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.3.2
)
FetchContent_MakeAvailable(nlohmann_json CLI11)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Define the executable
add_executable(armor
  ${SOURCES}
)

# Include project headers
target_include_directories(armor PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

# Use llvm_map_components_to_libnames to get only required LLVM libs
llvm_map_components_to_libnames(LLVM_LIBS
  support
  core
  option
)

# Link libraries
target_link_libraries(armor
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)

# Install the binary
install(TARGETS armor DESTINATION bin)

# CPack configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "armor")
set(CPACK_PACKAGE_VERSION "0.4.0")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "clang, clang-14, cmake, make, libclang-common-14-dev, libclang-14-dev, llvm-14-dev, libstdc++-11-dev, gcc-11, g++-11, git, build-essential")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Qualcomm API Backward Compatibility Checker
A static analysis tool for detecting API breakages between two versions of C/C++ public headers, classifying changes as backward compatible or incompatible.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.qualcomm.com/LinuxIntegration/API_Compatibility_Checker")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG")

include(CPack)

add_definitions(-DTOOL_VERSION="${CPACK_PACKAGE_VERSION}")
