cmake_minimum_required(VERSION 3.14)

# Find required packages
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# Map LLVM components to libraries
llvm_map_components_to_libnames(LLVM_LIBS
  support
  core
  option
)

# Source files
file(GLOB_RECURSE ALPHA_SOURCES "*.cpp")

# Define the executable
add_executable(alpha
  ${ALPHA_SOURCES}
)

# Enable testing mode
target_compile_definitions(alpha PRIVATE TESTING_ENABLED=1)

# Include project headers
target_include_directories(alpha PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/common/include
  ${CMAKE_SOURCE_DIR}/src/alpha/include
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

# Link libraries statically
target_link_libraries(alpha
  alpha_lib_test
  common_lib_test
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)

# Enable testing mode
target_compile_definitions(alpha PRIVATE TESTING_ENABLED=1)
