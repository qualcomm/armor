file(GLOB_RECURSE NSR_GENERATOR_DEBUG_SOURCES "src/*.cpp")

add_executable(nsr_generator_debug
  ${NSR_GENERATOR_DEBUG_SOURCES}
)

target_compile_definitions(nsr_generator_debug PRIVATE
  CLANG_FLAGS="${CLANG_EXTRA_FLAGS_CPP_ESCAPED}"
)

target_include_directories(nsr_generator_debug PRIVATE
  ${CMAKE_SOURCE_DIR}/src/common/include
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(nsr_generator_debug
  common_lib_test
  ${LLVM_LIBS}
  clangTooling
  clangIndex
  clangFrontend
)
