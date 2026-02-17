file(GLOB_RECURSE ALPHA_SOURCES "src/*.cpp")

add_library(alpha_lib STATIC
  ${ALPHA_SOURCES}
)

target_include_directories(alpha_lib PUBLIC
  ${CMAKE_SOURCE_DIR}/src/alpha/include
  ${CMAKE_SOURCE_DIR}/src/common/include
)

target_link_libraries(alpha_lib
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  common_lib
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)

add_library(alpha_lib_test STATIC
  ${ALPHA_SOURCES}
)

target_include_directories(alpha_lib_test PUBLIC
  ${CMAKE_SOURCE_DIR}/src/alpha/include
  ${CMAKE_SOURCE_DIR}/src/common/include
)

target_compile_definitions(alpha_lib_test PRIVATE TESTING_ENABLED=1)

target_link_libraries(alpha_lib_test
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  common_lib_test
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)