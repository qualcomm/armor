file(GLOB_RECURSE BETA_SOURCES "src/*.cpp")

add_library(beta_lib STATIC
  ${BETA_SOURCES}
)

target_include_directories(beta_lib PUBLIC
  ${CMAKE_SOURCE_DIR}/src/beta/include
  ${CMAKE_SOURCE_DIR}/src/common/include
)

target_link_libraries(beta_lib
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  common_lib
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)

add_library(beta_lib_test STATIC
  ${BETA_SOURCES}
)

target_include_directories(beta_lib_test PUBLIC
  ${CMAKE_SOURCE_DIR}/src/beta/include
  ${CMAKE_SOURCE_DIR}/src/common/include
)

target_compile_definitions(beta_lib_test PRIVATE TESTING_ENABLED=1)

target_link_libraries(beta_lib_test
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  common_lib_test
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)