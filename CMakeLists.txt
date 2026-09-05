cmake_minimum_required(VERSION 3.14)

project(YamlParser VERSION 2.0.0 LANGUAGES CXX)

set(YAMLPARSER_IS_TOP_LEVEL OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(YAMLPARSER_IS_TOP_LEVEL ON)
endif()

option(YAMLPARSER_BUILD_TESTS "Build the test suite" ${YAMLPARSER_IS_TOP_LEVEL})
option(YAMLPARSER_BUILD_EXAMPLES "Build usage examples" OFF)
option(YAMLPARSER_ENABLE_COVERAGE "Enable coverage instrumentation" OFF)
option(YAMLPARSER_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(yamlparser
  src/YamlFlowParser.cpp
  src/YamlParser.cpp
  src/YamlPrinter.cpp
  src/YamlScalarParser.cpp
  src/YamlValue.cpp
)
add_library(yamlparser::yamlparser ALIAS yamlparser)

target_compile_features(yamlparser PUBLIC cxx_std_14)
target_include_directories(yamlparser
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(yamlparser PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wshadow -Wconversion -Wsign-conversion
  )
elseif(MSVC)
  target_compile_options(yamlparser PRIVATE /W4 /WX /permissive-)
endif()

if(YAMLPARSER_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(yamlparser PRIVATE -fsanitize=address,undefined)
  target_link_options(yamlparser PRIVATE -fsanitize=address,undefined)
endif()

if(YAMLPARSER_ENABLE_COVERAGE)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    message(FATAL_ERROR "Coverage requires GCC or Clang")
  endif()
  target_compile_options(yamlparser PRIVATE -O0 -g --coverage)
  target_link_options(yamlparser PRIVATE --coverage)
endif()

if(YAMLPARSER_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

if(YAMLPARSER_BUILD_EXAMPLES)
  add_subdirectory(sample_usage)
endif()

if(YAMLPARSER_ENABLE_COVERAGE)
  include(${CMAKE_CURRENT_SOURCE_DIR}/CodeCoverage.cmake)
endif()

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
  file(GLOB_RECURSE YAMLPARSER_FORMAT_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sample_usage/src/*.cpp
  )
  add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${YAMLPARSER_FORMAT_FILES}
    COMMENT "Format C++ source files"
    VERBATIM
  )
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(TARGETS yamlparser
  EXPORT YamlParserTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT YamlParserTargets
  FILE YamlParserTargets.cmake
  NAMESPACE yamlparser::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/YamlParser
)

configure_package_config_file(
  cmake/YamlParserConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/YamlParserConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/YamlParser
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/YamlParserConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/YamlParserConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/YamlParserConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/YamlParser
)
