cmake_minimum_required(VERSION 3.14)

# Policy (quiet FetchContent timestamp if ever used)
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(yaml_parser_limitations LANGUAGES CXX)

# C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------
# Library visibility: reuse top-level 'yamlparser' if present
# ------------------------------------------------------------------
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
# Limitation samples
# ------------------------------------------------------------------
set(LIMITATION_SOURCES
  sample_test/merge_comment_test.cpp
  sample_test/nested_seq_test.cpp
  sample_test/escape_test.cpp
  sample_test/boolean_test.cpp
  sample_test/null_test.cpp
)

set(_LIM_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(LIMITATION_TARGETS "")
foreach(src ${LIMITATION_SOURCES})
  get_filename_component(name ${src} NAME_WE)

  add_executable(${name} ${src})
  target_link_libraries(${name} PRIVATE yamlparser)

  # Warnings + Debug-only sanitizers
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(${name} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    target_link_options   (${name} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
  elseif(MSVC)
    target_compile_options(${name} PRIVATE /W4 /permissive-)
  endif()

  # Put executables into limitation/bin for all configs
  set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY                 ${_LIM_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG           ${_LIM_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE         ${_LIM_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO  ${_LIM_BIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL      ${_LIM_BIN_DIR}
    # Handy when debugging in IDEs:
    VS_DEBUGGER_WORKING_DIRECTORY            ${CMAKE_CURRENT_SOURCE_DIR}
  )

  list(APPEND LIMITATION_TARGETS ${name})
endforeach()

message(STATUS "Limitation executables will be generated in: ${_LIM_BIN_DIR}")
message(STATUS "Limitation samples read YAML from: ${CMAKE_CURRENT_SOURCE_DIR}/sample_yaml/")

# ------------------------------------------------------------------
# Build limitation examples
# ------------------------------------------------------------------
add_custom_target(sample_limitation
  DEPENDS ${LIMITATION_TARGETS}
  COMMENT "Build all limitation examples (executables -> limitation/bin)"
)

# Run limitation examples from the limitation/ folder so relative paths work
add_custom_target(run_sample_limitation
  DEPENDS sample_limitation
  COMMAND ${CMAKE_COMMAND} -E echo "Running limitation examples..."
  COMMAND ${CMAKE_COMMAND} -E echo "================================"
  # Each run uses CWD = limitation/
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:merge_comment_test>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:nested_seq_test>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:escape_test>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:boolean_test>
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:null_test>
  USES_TERMINAL
  COMMENT "Run all limitation executables from 'limitation/'"
)
