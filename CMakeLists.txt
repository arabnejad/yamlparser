# Set CMP0135 policy to NEW to avoid FetchContent timestamp warning
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
# Clang-format target for code style enforcement
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
    add_custom_target(
        clang_format
        COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-format on all source and header files in src, include, tests, limitation, and sample_usage"
    )
else()
    message(WARNING "clang-format not found! Code formatting target will be unavailable.")
endif()



cmake_minimum_required(VERSION 3.14)
project(YamlParserLib VERSION 1.0.0 LANGUAGES CXX)

# Set policy to suppress FetchContent timestamp warning
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()



set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



# Option for enabling code coverage
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

# Automatically find all .cpp files in src/ and add them as sources
file(GLOB YAML_PARSER_SOURCES src/*.cpp)
add_library(yamlparser ${YAML_PARSER_SOURCES})

target_include_directories(yamlparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add code coverage flags and targets if enabled
if(ENABLE_COVERAGE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/CodeCoverage.cmake)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(yamlparser PRIVATE -O0 -g --coverage)
        target_link_options(yamlparser PRIVATE --coverage)
    endif()
endif()


# Maximum compile-time checking: treat all warnings as errors
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    # Strict standards compliance, maximum warnings as errors, and sanitizers
    target_compile_options(yamlparser PRIVATE
        -Wall -Wextra -Werror -Wpedantic
        -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual
        -Wconversion -Wsign-conversion -Wmisleading-indentation
        -Wnull-dereference -Wdouble-promotion
        -fsanitize=address,undefined
    )
    target_link_options(yamlparser PRIVATE -fsanitize=address,undefined)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Strict standards compliance and warnings as errors
    target_compile_options(yamlparser PRIVATE /W4 /WX /permissive-)
endif()

# Install rules
install(TARGETS yamlparser DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

# Export for use as a third-party dependency
include(CMakePackageConfigHelpers)
write_basic_config_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/YamlParserLibConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/YamlParserLibConfigVersion.cmake"
    DESTINATION lib/cmake/YamlParserLib)

# Option to enable/disable testing (default ON)
option(ENABLE_UNIT_TESTS "Enable building and running unit tests" ON)

if(ENABLE_UNIT_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

# --- Doxygen documentation target with doxygen-awesome-css ---
find_package(Doxygen)
include(FetchContent)
FetchContent_Declare(
    doxygen-awesome-css
    URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(doxygen-awesome-css)
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

if(DOXYGEN_FOUND)
    add_custom_target(doxyget
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen and doxygen-awesome-css"
        VERBATIM
    )
else()
    message(WARNING "Doxygen not found. Documentation target 'doxyget' will not be available.")
endif()

# Coverage settings only if enabled
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(STATUS "Enabling coverage flags for GCC/Clang.")
        set(COVERAGE_FLAGS "-O0 -g --coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_FLAGS}")
    endif()
endif()

