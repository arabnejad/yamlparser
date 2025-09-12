cmake_minimum_required(VERSION 3.14)
project(YamlParserTests LANGUAGES CXX)

# Policies
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW) # suppress FetchContent timestamp warnings
endif()

# We expect the library target to exist (added from the root via add_subdirectory)
if(NOT TARGET yamlparser)
  message(FATAL_ERROR "Target 'yamlparser' not found. Run from the project root where it is defined.")
endif()

# ------------------------------------------------------------------
# GoogleTest via FetchContent
# ------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # MSVC nicety
FetchContent_MakeAvailable(googletest)

# ------------------------------------------------------------------
# Test assets: copy YAML files next to the test binary
# ------------------------------------------------------------------
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_cases)
file(GLOB TEST_CASE_YAMLS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test_cases/*.yaml")
foreach(yaml_file IN LISTS TEST_CASE_YAMLS)
  configure_file(${yaml_file} ${CMAKE_CURRENT_BINARY_DIR}/test_cases/ COPYONLY)
endforeach()

# ------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(yamlparser_gtest ${TEST_SOURCES})

# ------------------------------------------------------------------
# Link: library + GTest (namespaced targets)
# ------------------------------------------------------------------
target_link_libraries(yamlparser_gtest PRIVATE yamlparser GTest::gtest GTest::gtest_main)


# ------------------------------------------------------------------
# Warnings / Sanitizers (Debug only)
# ------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(yamlparser_gtest PRIVATE
    -Wall -Wextra -Wpedantic
    -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual
    -Wconversion -Wsign-conversion -Wmisleading-indentation
    -Wnull-dereference -Wdouble-promotion
  )
  target_compile_options(yamlparser_gtest PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
  target_link_options   (yamlparser_gtest PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
elseif(MSVC)
  target_compile_options(yamlparser_gtest PRIVATE /W4 /permissive-)
endif()

# ------------------------------------------------------------------
# Coverage (optional, mirrors root option)
# ------------------------------------------------------------------
if(DEFINED ENABLE_COVERAGE AND ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(yamlparser_gtest PRIVATE -O0 -g --coverage)
    target_link_options   (yamlparser_gtest PRIVATE --coverage)
  endif()
endif()

# ------------------------------------------------------------------
# Register tests; run from tests' build dir so relative paths work
# ------------------------------------------------------------------
include(GoogleTest)
gtest_discover_tests(yamlparser_gtest
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DISCOVERY_TIMEOUT 60
)

# ------------------------------------------------------------------
# Build Tests
# ------------------------------------------------------------------
# Build Tests
add_custom_target(tests
  DEPENDS yamlparser_gtest
  COMMENT "Build all unit test binaries"
)

# ------------------------------------------------------------------
# Run Tests (via CTest)
# ------------------------------------------------------------------
add_custom_target(run_tests_ctest
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
  DEPENDS tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Run all unit tests via ctest"
)

# ------------------------------------------------------------------
# Run Tests (directly, without CTest)
# ------------------------------------------------------------------
add_custom_target(run_tests
  COMMAND $<TARGET_FILE:yamlparser_gtest>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Run tests directly (no CTest, no build/Testing/ folder)"
)