cmake_minimum_required(VERSION 3.14)


# Set policy to suppress FetchContent timestamp warning
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

project(yaml_parser_limitations)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use FetchContent to include the yamlparser library
include(FetchContent)




# Always add yamlparser as a static library from local source
add_library(yamlparser STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/YamlParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/YamlElement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/YamlHelperFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/YamlPrinter.cpp
)
target_include_directories(yamlparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Define test executables
set(TEST_SOURCES
    sample_test/merge_comment_test.cpp
    sample_test/nested_seq_test.cpp
    sample_test/escape_test.cpp
    sample_test/boolean_test.cpp
    sample_test/scientific_test.cpp
    sample_test/null_test.cpp
)

# Create executables for each test
foreach(TEST_SOURCE ${TEST_SOURCES})
    # Extract the test name from the file path
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    # Create executable in the same directory as the source file
    get_filename_component(TEST_DIR ${TEST_SOURCE} DIRECTORY)

    add_executable(${TEST_NAME} ${TEST_SOURCE})

    # Link against the yamlparser library
    target_link_libraries(${TEST_NAME} PRIVATE yamlparser)

    # Match the compiler flags from the main project for compatibility
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${TEST_NAME} PRIVATE
            -Wall -Wextra -Wpedantic
            -fsanitize=address,undefined
        )
        target_link_options(${TEST_NAME} PRIVATE -fsanitize=address,undefined)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${TEST_NAME} PRIVATE /W4 /permissive-)
    endif()

    # Set the output directory to be the same as the source directory
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIR}
    )

    # Set working directory for tests to be the limitation folder
    set_target_properties(${TEST_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

# Print build information
message(STATUS "Building YAML Parser Limitation Tests")
message(STATUS "Test executables will be generated in: ${CMAKE_CURRENT_SOURCE_DIR}/sample_test/")
message(STATUS "Tests will read YAML files from: ${CMAKE_CURRENT_SOURCE_DIR}/sample_yaml/")

# Custom target to build all tests
add_custom_target(all_limitation_tests ALL DEPENDS
    merge_comment_test
    nested_seq_test
    escape_test
    boolean_test
    scientific_test
    null_test
)
