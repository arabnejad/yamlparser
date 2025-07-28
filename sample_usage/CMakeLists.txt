cmake_minimum_required(VERSION 3.14)


# Set policy to suppress FetchContent timestamp warning
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

project(YamlParserSamples)

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

# Function to create an executable for each sample
function(add_sample_executable sample_name)
    add_executable(${sample_name} src/${sample_name}.cpp)

    # Link against the yamlparser library
    target_link_libraries(${sample_name} PRIVATE yamlparser)

    # Match the compiler flags from the main project for compatibility
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${sample_name} PRIVATE
            -Wall -Wextra -Wpedantic
            -fsanitize=address,undefined
        )
        target_link_options(${sample_name} PRIVATE -fsanitize=address,undefined)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${sample_name} PRIVATE /W4 /permissive-)
    endif()

    # Set output directory to bin/
    set_target_properties(${sample_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endfunction() # Add all sample executables
add_sample_executable(basic_config)
add_sample_executable(nested_structures)
add_sample_executable(arrays_sequences)
add_sample_executable(anchors_merge)
add_sample_executable(nested_maps)
add_sample_executable(multiline_strings)
add_sample_executable(data_types)
add_sample_executable(app_config)
add_sample_executable(complex_data)
add_sample_executable(nested_arrays)

# Custom target to build all samples
add_custom_target(all_samples
    DEPENDS
    basic_config
    nested_structures
    arrays_sequences
    anchors_merge
    nested_maps
    multiline_strings
    data_types
    app_config
    complex_data
    nested_arrays
)

# Custom target to run all samples
add_custom_target(run_all_samples
    COMMAND echo "Running all YAML parser samples..."
    COMMAND echo "=================================="
    COMMAND echo ""
    COMMAND echo "1. Basic Configuration:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/basic_config || true
    COMMAND echo ""
    COMMAND echo "2. Nested Structures:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/nested_structures || true
    COMMAND echo ""
    COMMAND echo "3. Arrays and Sequences:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/arrays_sequences || true
    COMMAND echo ""
    COMMAND echo "4. Anchors and Merge Keys:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/anchors_merge || true
    COMMAND echo ""
    COMMAND echo "5. Nested Maps:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/nested_maps || true
    COMMAND echo ""
    COMMAND echo "6. Multiline Strings:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/multiline_strings || true
    COMMAND echo ""
    COMMAND echo "7. Data Types:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/data_types || true
    COMMAND echo ""
    COMMAND echo "8. Application Configuration:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/app_config || true
    COMMAND echo ""
    COMMAND echo "9. Complex Data Structures:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/complex_data || true
    COMMAND echo ""
    COMMAND echo "10. Nested Arrays:"
    COMMAND cd ${CMAKE_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/bin/nested_arrays || true
    COMMAND echo ""
    COMMAND echo "All samples completed!"
    DEPENDS all_samples
)

# Print information about available targets
message(STATUS "Sample Usage Examples Configuration:")
message(STATUS "====================================")
message(STATUS "Available targets:")
message(STATUS "  all_samples     - Build all sample executables")
message(STATUS "  run_all_samples - Build and run all samples")
message(STATUS "  <sample_name>   - Build individual sample")
message(STATUS "")
message(STATUS "Individual samples:")
message(STATUS "  basic_config       - Basic YAML configuration parsing")
message(STATUS "  nested_structures  - Nested map and value extraction")
message(STATUS "  arrays_sequences   - Array and sequence processing")
message(STATUS "  anchors_merge      - Anchors and merge keys (limited)")
message(STATUS "  nested_maps        - Deep nested map navigation")
message(STATUS "  multiline_strings  - Multiline string handling")
message(STATUS "  data_types         - Various YAML data types")
message(STATUS "  app_config         - Real-world app configuration")
message(STATUS "  complex_data       - Complex nested data structures")
message(STATUS "  nested_arrays      - Multi-dimensional arrays")
message(STATUS "")
message(STATUS "Usage:")
message(STATUS "  cmake -S . -B build")
message(STATUS "  cmake --build build")
message(STATUS "  cmake --build build --target run_all_samples")
