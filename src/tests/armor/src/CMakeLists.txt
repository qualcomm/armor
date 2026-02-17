# Source files
file(GLOB_RECURSE ARMOR_DEBUG_SOURCES "*.cpp")

# Define the executable
add_executable(armor_debug
  ${ARMOR_DEBUG_SOURCES}
)

# Enable testing mode
target_compile_definitions(armor_debug PRIVATE TESTING_ENABLED=1)

# Include project headers
target_include_directories(armor_debug PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/armor/include
  ${CMAKE_SOURCE_DIR}/src/common/include
  ${CMAKE_SOURCE_DIR}/src/alpha/include
  ${CMAKE_SOURCE_DIR}/src/beta/include
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

# Link libraries statically
target_link_libraries(armor_debug
  alpha_lib_test
  beta_lib_test
  common_lib_test
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)
