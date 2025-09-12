# CodeCoverage.cmake — define coverage targets driven by gcovr
#
# Usage:
#   - Enable coverage flags in your build (e.g. ENABLE_COVERAGE=ON in your root).
#   - Include this file from the root CMakeLists.txt *after* tests are added:
#       include(${CMAKE_SOURCE_DIR}/CodeCoverage.cmake)
#   - Targets provided:
#       gcovr_html     -> writes ${CMAKE_BINARY_DIR}/coverage/index.html
#       gcovr_console  -> prints summary to terminal
#       gcovr_xml      -> writes ${CMAKE_BINARY_DIR}/coverage/coverage.xml
#       gcovr_clean    -> removes ${CMAKE_BINARY_DIR}/coverage
#
# Notes:
#   - Only active when using GCC/Clang AND ENABLE_COVERAGE is ON.
#   - Automatically depends on 'run_tests' (or 'tests') if that target exists.
#   - Excludes common build/system paths and tests/ by default.


# ------------------------------------------------------------------
# Compiler / option gate
# ------------------------------------------------------------------
if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
  message(STATUS "Coverage: supported only with GCC/Clang; skipping coverage targets.")
  return()
endif()

if(NOT ENABLE_COVERAGE)
  message(STATUS "Coverage: ENABLE_COVERAGE is OFF; skipping coverage targets.")
  return()
endif()

# ------------------------------------------------------------------
# Tools
# ------------------------------------------------------------------
find_program(GCOVR gcovr)
if(NOT GCOVR)
  message(FATAL_ERROR "Coverage: 'gcovr' not found. Install gcovr or disable ENABLE_COVERAGE.")
endif()

# ------------------------------------------------------------------
# Output dir
# ------------------------------------------------------------------
set(COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage" CACHE PATH "Coverage output directory")

# ------------------------------------------------------------------
# Base args
# ------------------------------------------------------------------
set(_gcovr_base_args
  --root "${CMAKE_SOURCE_DIR}"
  --filter "${CMAKE_SOURCE_DIR}"
  --exclude-unreachable-branches
  --exclude-throw-branches
)

# ------------------------------------------------------------------
# Default excludes (regex)
# ------------------------------------------------------------------
set(_coverage_excludes_default
  "${CMAKE_BINARY_DIR}/_deps/.*"
  ".*/_deps/.*"
  ".*/CMakeFiles/.*"
  "${CMAKE_SOURCE_DIR}/tests/.*"
  "/Library/.*"
  "/usr/.*"
  ".*YamlException.*"
)

# ------------------------------------------------------------------
# Allow user to add extra excludes from the cache as a space-separated string
# ------------------------------------------------------------------
set(COVERAGE_EXCLUDES "" CACHE STRING "Extra gcovr --exclude regexes (space-separated)")
separate_arguments(COVERAGE_EXCLUDES)

# ------------------------------------------------------------------
# Compose final exclude args
# ------------------------------------------------------------------
set(_gcovr_exclude_args "")
foreach(rx IN LISTS _coverage_excludes_default COVERAGE_EXCLUDES)
  list(APPEND _gcovr_exclude_args --exclude "${rx}")
endforeach()

# ------------------------------------------------------------------
# Choose a prerequisite if present (so coverage actually runs tests first)
# ------------------------------------------------------------------
set(_coverage_prereq "")
if(TARGET run_tests)
  set(_coverage_prereq run_tests)
elseif(TARGET tests)
  set(_coverage_prereq tests)
endif()

# ------------------------------------------------------------------
# HTML report
# ------------------------------------------------------------------
add_custom_target(gcovr_html
  COMMAND ${CMAKE_COMMAND} -E make_directory "${COVERAGE_OUTPUT_DIR}"
  COMMAND ${GCOVR} ${_gcovr_base_args} ${_gcovr_exclude_args}
          --html --html-details
          -o "${COVERAGE_OUTPUT_DIR}/index.html"
  DEPENDS ${_coverage_prereq}
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Generating HTML coverage report at ${COVERAGE_OUTPUT_DIR}/index.html"
  VERBATIM
)

# ------------------------------------------------------------------
# Console summary
# ------------------------------------------------------------------
add_custom_target(gcovr_console
  COMMAND ${GCOVR} ${_gcovr_base_args} ${_gcovr_exclude_args}
  DEPENDS ${_coverage_prereq}
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Generating console coverage report"
  VERBATIM
)

# ------------------------------------------------------------------
# Cobertura XML (CI integration)
# ------------------------------------------------------------------
add_custom_target(gcovr_xml
  COMMAND ${CMAKE_COMMAND} -E make_directory "${COVERAGE_OUTPUT_DIR}"
  COMMAND ${GCOVR} ${_gcovr_base_args} ${_gcovr_exclude_args}
          --xml -o "${COVERAGE_OUTPUT_DIR}/coverage.xml"
  DEPENDS ${_coverage_prereq}
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Generating Cobertura XML at ${COVERAGE_OUTPUT_DIR}/coverage.xml"
  VERBATIM
)

# ------------------------------------------------------------------
# Clean coverage artifacts only
# ------------------------------------------------------------------
add_custom_target(gcovr_clean
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${COVERAGE_OUTPUT_DIR}"
  COMMENT "Remove coverage output directory ${COVERAGE_OUTPUT_DIR}"
  VERBATIM
)
