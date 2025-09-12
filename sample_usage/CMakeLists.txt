cmake_minimum_required(VERSION 3.14)

# Policies
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW) # suppress FetchContent timestamp warnings
endif()

# Project
project(YamlParserSamples LANGUAGES CXX)

# Language standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------
# Library visibility
# ------------------------------------------------------------------
# When built standalone, create the library; when included from root,
# reuse the existing 'yamlparser' target.
if(NOT TARGET yamlparser)
  add_library(yamlparser STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/YamlParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/YamlElement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/YamlHelperFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/YamlPrinter.cpp
  )
  target_include_directories(yamlparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
endif()

# ------------------------------------------------------------------
# Helper: add one sample executable with consistent flags/output
# ------------------------------------------------------------------
function(add_sample_executable sample_name)
  add_executable(${sample_name} src/${sample_name}.cpp)
  target_link_libraries(${sample_name} PRIVATE yamlparser)

  # Warnings/sanitizers (Debug-only suggested)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${sample_name} PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(${sample_name} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    target_link_options   (${sample_name} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
  elseif(MSVC)
    target_compile_options(${sample_name} PRIVATE /W4 /permissive-)
  endif()

  # Put executables in the SOURCE tree: sample_usage
  set_target_properties(${sample_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY                 ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG           ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE         ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO  ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL      ${CMAKE_CURRENT_SOURCE_DIR}/bin
  )
endfunction()

# ------------------------------------------------------------------
# Samples
# ------------------------------------------------------------------
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

# ------------------------------------------------------------------
# Build-all aggregator (kept for clarity)
# ------------------------------------------------------------------
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

# Requested name to build all samples
add_custom_target(sample_usage
  DEPENDS all_samples
  COMMENT "Build all sample usage executables"
)

# ------------------------------------------------------------------
# Run all samples from the sample_usage/ source dir so 'yaml_files/...' works.
# cmake -E chdir scopes the CWD only for that one command (no need to cd back).
# ------------------------------------------------------------------
add_custom_target(run_all_sample_usage
  DEPENDS all_samples
  COMMAND ${CMAKE_COMMAND} -E echo "Running all YAML parser samples..."
  COMMAND ${CMAKE_COMMAND} -E echo "=================================="

  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:basic_config>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:nested_structures>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:arrays_sequences>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:anchors_merge>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:nested_maps>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:multiline_strings>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:data_types>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:app_config>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:complex_data>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:nested_arrays>

  USES_TERMINAL
  COMMENT "Run all sample executables from sample_usage/"
)

# legacy alias for backward compatibility
add_custom_target(run_all_samples
  DEPENDS run_all_sample_usage
  COMMENT "Alias: use 'run_all_sample_usage'"
)
