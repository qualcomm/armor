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
file(GLOB_RECURSE ARMOR_SOURCES "*.cpp")

# Define the executable
set(TOOL_VERSION "0.5.1")
add_executable(armor
  ${ARMOR_SOURCES}
)

# Include project headers
target_include_directories(armor PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/armor/include
  ${CMAKE_SOURCE_DIR}/src/common/include
  ${CMAKE_SOURCE_DIR}/src/alpha/include
  ${CMAKE_SOURCE_DIR}/src/beta/include
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

# Define compile definitions
target_compile_definitions(armor PRIVATE
  TOOL_VERSION="${TOOL_VERSION}"
)

# Link libraries statically
target_link_libraries(armor
  alpha_lib
  beta_lib
  common_lib
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)
