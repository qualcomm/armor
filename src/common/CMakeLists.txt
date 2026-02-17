include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)

FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.3.2
)

FetchContent_MakeAvailable(nlohmann_json CLI11)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(common_lib STATIC
  ${SOURCES}
)

target_include_directories(common_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(common_lib
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)

add_library(common_lib_test STATIC
  ${SOURCES}
)

target_include_directories(common_lib_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(common_lib_test PRIVATE TESTING_ENABLED=1)

target_link_libraries(common_lib_test
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  nlohmann_json::nlohmann_json
  CLI11::CLI11
)