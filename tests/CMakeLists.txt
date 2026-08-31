include(FetchContent)

find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(yamlparser_tests ${TEST_SOURCES})
target_link_libraries(yamlparser_tests PRIVATE yamlparser::yamlparser GTest::gtest_main)
target_compile_features(yamlparser_tests PRIVATE cxx_std_14)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(yamlparser_tests PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
elseif(MSVC)
  target_compile_options(yamlparser_tests PRIVATE /W4 /permissive-)
endif()

if(YAMLPARSER_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(yamlparser_tests PRIVATE -fsanitize=address,undefined)
  target_link_options(yamlparser_tests PRIVATE -fsanitize=address,undefined)
endif()

if(YAMLPARSER_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(yamlparser_tests PRIVATE -O0 -g --coverage)
  target_link_options(yamlparser_tests PRIVATE --coverage)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_cases DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include(GoogleTest)
gtest_discover_tests(yamlparser_tests
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DISCOVERY_TIMEOUT 60
)

add_custom_target(run_tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  DEPENDS yamlparser_tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Run the YAML parser tests"
)
