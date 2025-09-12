cmake_minimum_required(VERSION 3.14)

# Policies
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)  # Avoid FetchContent timestamp warning
endif()

# Disallow in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not supported. Use: cmake -S . -B build")
endif()

# Project
project(YamlParserLib VERSION 1.0.0 LANGUAGES CXX)

# Language standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(ENABLE_UNIT_TESTS "Enable building and running unit tests" ON)
option(ENABLE_COVERAGE   "Enable coverage reporting"              ON)

# ------------------------------------------------------------------
# Tooling: clang-format
# ------------------------------------------------------------------
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
  file(GLOB ALL_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/limitation/sample_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sample_usage/src/*.cpp
  )
  add_custom_target(clang_format
    COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running clang-format on all source and header files"
  )
else()
  message(WARNING "clang-format not found! Target 'clang_format' unavailable.")
endif()

# ------------------------------------------------------------------
# YamlParser Library
# ------------------------------------------------------------------
file(GLOB YAML_PARSER_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(yamlparser ${YAML_PARSER_SOURCES})
target_include_directories(yamlparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Provide requested alias name
add_library(yamlParserLib ALIAS yamlparser)

# ------------------------------------------------------------------
# Warnings / Sanitizers (Debug only)
# ------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(yamlparser PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual
    -Wconversion -Wsign-conversion -Wmisleading-indentation
    -Wnull-dereference -Wdouble-promotion
  )
  # Only enable sanitizers in Debug builds
  target_compile_options(yamlparser PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
  target_link_options   (yamlparser PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(yamlparser PRIVATE /W4 /WX /permissive-)
endif()

# ------------------------------------------------------------------
# Coverage (if enabled)
# ------------------------------------------------------------------
if(ENABLE_COVERAGE)
  include(${CMAKE_CURRENT_SOURCE_DIR}/CodeCoverage.cmake)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Code coverage enabled")
    target_compile_options(yamlparser PRIVATE -O0 -g --coverage)
    target_link_libraries(yamlparser PRIVATE --coverage)
    target_link_options(yamlparser PRIVATE --coverage)
  else()
    message(WARNING "Code coverage is only supported with GCC or Clang")
  endif()
endif()

# ------------------------------------------------------------------
# Install / Package config
# ------------------------------------------------------------------
install(TARGETS yamlparser DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
# Export for use as a third-party dependency
include(CMakePackageConfigHelpers)
# Write the version file into the *current* binary dir (e.g. build/)
write_basic_config_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/YamlParserLibConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)
# Install it
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/YamlParserLibConfigVersion.cmake"
  DESTINATION lib/cmake/YamlParserLib
)

# ------------------------------------------------------------------
# Subdirectories: examples & tests
# ------------------------------------------------------------------

# Add samples usage (make their targets visible at the root)
add_subdirectory(sample_usage)
# Add Limitation examples (make their targets visible at the root)
add_subdirectory(limitation)

# Tests
if(ENABLE_UNIT_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

# ------------------------------------------------------------------
# Docs (Doxygen)
# ------------------------------------------------------------------
find_package(Doxygen)
if(DOXYGEN_FOUND)
  include(FetchContent)
  # Only fetch CSS helper if doxygen is present
  FetchContent_Declare(
    doxygen-awesome-css
    URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
  )
  FetchContent_MakeAvailable(doxygen-awesome-css)
  FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

  set(DOXYFILE_IN  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation (Doxygen + doxygen-awesome-css)"
    VERBATIM
  )
else()
  message(STATUS "Doxygen not found. Target 'docs' will not be available.")
endif()

# ------------------------------------------------------------------
# Clean-all helper (remove all CMake build trees under the repo)
# ------------------------------------------------------------------
add_custom_target(clean_all
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}
          ${CMAKE_COMMAND} -DROOT=${CMAKE_SOURCE_DIR}
                           -P ${CMAKE_SOURCE_DIR}/clean_all.cmake
  COMMENT "Remove ALL CMake build directories under the source tree"
  USES_TERMINAL
  VERBATIM
)