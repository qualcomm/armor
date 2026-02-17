include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG  v1.14.0
)

FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB TEST_SOURCES "unit/*.cpp")

add_executable(common_unit_tests
  ${TEST_SOURCES}
)

target_include_directories(common_unit_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src/common/include
  ${CMAKE_SOURCE_DIR}/src/alpha/include
  ${CMAKE_SOURCE_DIR}/src/beta/include
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(common_unit_tests
  gtest
  gtest_main
  common_lib_test
  ${LLVM_LIBS}
)

include(GoogleTest)
gtest_discover_tests(common_unit_tests)
