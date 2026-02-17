# Source files
file(GLOB_RECURSE ARMOR_SOURCES "*.cpp")

# Define the executable
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

install(TARGETS armor DESTINATION bin)
