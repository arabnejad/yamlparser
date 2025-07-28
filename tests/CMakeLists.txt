# Test CMakeLists.txt for yamlparser
cmake_minimum_required(VERSION 3.14)
project(YamlParserTests)


include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Create test_cases directory and copy YAML files
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_cases)
file(GLOB TEST_CASE_YAMLS "${CMAKE_CURRENT_SOURCE_DIR}/test_cases/*.yaml")
foreach(yaml_file ${TEST_CASE_YAMLS})
    configure_file(${yaml_file} ${CMAKE_CURRENT_BINARY_DIR}/test_cases/ COPYONLY)
endforeach()



# Automatically find all .cpp files in tests/ and add them as sources
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(yamlparser_gtest ${TEST_SOURCES})


# Add compiler flags and options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(COMMON_FLAGS
        -Wall -Wextra -Werror -Wpedantic
        -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual
        -Wconversion -Wsign-conversion -Wmisleading-indentation
        -Wnull-dereference -Wdouble-promotion
        -fsanitize=address,undefined)

    if(ENABLE_COVERAGE)
        list(APPEND COMMON_FLAGS -O0 -g --coverage)
        target_link_options(yamlparser_gtest PRIVATE --coverage)
    endif()

    target_compile_options(yamlparser_gtest PRIVATE ${COMMON_FLAGS})
    target_link_options(yamlparser_gtest PRIVATE -fsanitize=address,undefined)
endif()
target_include_directories(yamlparser_gtest PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(yamlparser_gtest PRIVATE yamlparser gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(yamlparser_gtest)


