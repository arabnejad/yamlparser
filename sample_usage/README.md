\page sample_usage Sample Usage Examples
\tableofcontents

# YAML Parser Sample Usage Examples

This folder contains examples demonstrating how to use the YAML Parser library in various real-world scenarios. Each example includes both a C++ implementation and corresponding YAML files to show practical usage patterns.

## Overview

The sample usage examples cover a wide range of YAML parsing scenarios, from basic configuration files to complex nested data structures. These examples are designed to help you understand how to effectively use the YAML Parser library in your own projects.

## Examples Included

### 1. Basic Configuration (`basic_config`)
**Files**: `src/basic_config.cpp`, `yaml_files/basic_config.yaml`

Demonstrates fundamental YAML parsing operations:
- Reading simple key-value pairs
- Extracting strings, integers, doubles, and booleans
- Basic error handling
- Type checking and conversion

### 2. Nested Structures (`nested_structures`)
**Files**: `src/nested_structures.cpp`, `yaml_files/nested_structures.yaml`

Shows how to navigate nested maps:
- Accessing deeply nested values
- Traversing hierarchical configurations
- Handling missing keys gracefully
- Working with multi-level data structures

### 3. Arrays and Sequences (`arrays_sequences`)
**Files**: `src/arrays_sequences.cpp`, `yaml_files/arrays_sequences.yaml`

Covers array processing techniques:
- Iterating through sequences
- Processing mixed-type arrays
- Handling arrays of objects
- Working with different data types in sequences

### 4. Anchors and Merge Keys (`anchors_merge`)
**Files**: `src/anchors_merge.cpp`, `yaml_files/anchors_merge.yaml`

Demonstrates anchor and merge key usage:
- Understanding parser limitations with anchors
- Working with reference and merge syntax
- Best practices for repetitive data
- **Note**: This parser has limitations with full anchor/merge support

### 5. Nested Maps (`nested_maps`)
**Files**: `src/nested_maps.cpp`, `yaml_files/nested_maps.yaml`

Advanced map navigation techniques:
- Deep hierarchical data structures
- Recursive map traversal
- Complex configuration parsing
- Multi-level data extraction

### 6. Multiline Strings (`multiline_strings`)
**Files**: `src/multiline_strings.cpp`, `yaml_files/multiline_strings.yaml`

Multiline string handling:
- Literal style (`|`) strings
- Folded style (`>`) strings
- Quoted strings with escapes
- Text formatting preservation
- **Note**: Parser may have limitations with some multiline styles

### 7. Data Types (`data_types`)
**Files**: `src/data_types.cpp`, `yaml_files/data_types.yaml`

Data type demonstration:
- Strings, integers, floats, booleans
- Special values (null, infinity, NaN)
- Date and timestamp parsing
- Type detection and conversion
- **Note**: Some special values may be parsed as strings

### 8. Application Configuration (`app_config`)
**Files**: `src/app_config.cpp`, `yaml_files/app_config.yaml`

Real-world application configuration:
- Database connection settings
- Server configuration
- Logging setup
- Feature flags
- Cache configuration

### 9. Complex Data Structures (`complex_data`)
**Files**: `src/complex_data.cpp`, `yaml_files/complex_data.yaml`

Advanced nested data handling:
- Company organizational data
- Employee and department structures
- Project management data
- Multi-level object relationships

### 10. Nested Arrays (`nested_arrays`)
**Files**: `src/nested_arrays.cpp`, `yaml_files/nested_arrays.yaml`

Multi-dimensional array processing:
- 2D matrices
- 3D tensors
- Mixed nested structures
- Game boards and coordinate systems

## Building and Running

### Prerequisites
- CMake 3.10 or higher
- C++14 compatible compiler
- Access to the main YAML Parser source files (in `../src/` and `../include/`)

### Build Instructions

1. **Configure the build**:
   ```bash
   cmake -S . -B build
   ```

2. **Build all samples**:
   ```bash
   cmake --build build
   ```

3. **Build a specific sample**:
   ```bash
   cmake --build build --target basic_config
   ```

4. **Run all samples**:
   ```bash
   cmake --build build --target run_all_samples
   ```

### Running Individual Examples

After building, executables are located in `build/bin/`:

```bash
# Run individual examples
./build/bin/basic_config
./build/bin/nested_structures
./build/bin/arrays_sequences
# ... and so on
```

## Project Structure

```
sample_usage/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This documentation
├── src/                        # C++ source files
│   ├── basic_config.cpp
│   ├── nested_structures.cpp
│   ├── arrays_sequences.cpp
│   ├── anchors_merge.cpp
│   ├── nested_maps.cpp
│   ├── multiline_strings.cpp
│   ├── data_types.cpp
│   ├── app_config.cpp
│   ├── complex_data.cpp
│   └── nested_arrays.cpp
└── yaml_files/                 # Example YAML files
    ├── basic_config.yaml
    ├── nested_structures.yaml
    ├── arrays_sequences.yaml
    ├── anchors_merge.yaml
    ├── nested_maps.yaml
    ├── multiline_strings.yaml
    ├── data_types.yaml
    ├── app_config.yaml
    ├── complex_data.yaml
    └── nested_arrays.yaml
```

## Usage Patterns

### Basic Usage Pattern
```cpp
#include "YamlParser.hpp"
#include "YamlException.hpp"

using namespace yamlparser;

int main() {
    try {
        YamlParser parser;
        parser.parse("config.yaml");

        if (!parser.isSequenceRoot()) {
            const auto& config = parser.root();
            // Process your YAML data here
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
```### Type Checking and Safe Access
```cpp
// Always check types before accessing values
if (config.count("key") && config.at("key").value.isString()) {
    std::string value = config.at("key").value.asString();
}

// Handle nested structures safely
if (config.count("section") && config.at("section").value.isMap()) {
    const auto& section = config.at("section").value.asMap();
    // Process nested map
}
```

### Error Handling Best Practices
```cpp
try {
    YamlParser parser;
    parser.parse("config.yaml");
    // Process document
} catch (const std::exception& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
    // Handle error appropriately
}
```

## Integration with Your Project

### Method 1: Copy Source Files
To integrate these examples into your own project by copying source files:

1. **Copy the parser source files** to your project:
   ```bash
   # Copy header files
   cp -r ../include/ your_project/include/

   # Copy source files
   cp -r ../src/ your_project/src/
   ```

2. **Update your CMakeLists.txt**:
   ```cmake
   # Create yamlparser library
   add_library(yamlparser STATIC
       src/YamlParser.cpp
       src/YamlElement.cpp
       src/YamlHelperFunctions.cpp
       src/YamlPrinter.cpp
   )

   target_include_directories(yamlparser PUBLIC include)

   # Link your executable
   add_executable(your_app main.cpp)
   target_link_libraries(your_app PRIVATE yamlparser)
   ```

3. **Use standard includes** in your code:
   ```cpp
   #include "YamlParser.hpp"
   ```

### Method 2: CMake FetchContent (Recommended)
For the most modern approach, use CMake FetchContent:

```cmake
include(FetchContent)
FetchContent_Declare(
    yamlparser
    GIT_REPOSITORY https://github.com/arabnejad/yamlparser.git
    GIT_TAG master  # or specific tag/commit
)
FetchContent_MakeAvailable(yamlparser)

add_executable(your_app main.cpp)
target_link_libraries(your_app PRIVATE yamlparser)
```

### Method 3: Git Submodule
Add as a git submodule:

```bash
git submodule add https://github.com/arabnejad/yamlparser.git third_party/yamlparser
```

Then in your CMakeLists.txt:
```cmake
add_subdirectory(third_party/yamlparser)
target_link_libraries(your_app PRIVATE yamlparser)
```

### Key Benefits of This Approach:

1. **Standard Include Syntax**: Use `#include "YamlParser.hpp"` instead of relative paths
2. **Proper CMake Targets**: Uses modern target-based CMake with `target_link_libraries`
3. **Portable**: No hardcoded paths - works anywhere the library is installed
4. **Scalable**: Easy to integrate into larger projects
5. **Best Practices**: Follows modern C++ and CMake conventions

1. **Copy the source files** you need as templates
2. **Modify the YAML file paths** to match your project structure
3. **Adapt the parsing logic** to your specific data structures
4. **Add error handling** appropriate for your use case
5. **Consider the parser limitations** noted in individual examples

## Parser Limitations

Please be aware of the following limitations when using these examples:

- **Anchors and Aliases**: Limited support for `&` and `*` syntax
- **Merge Keys**: Limited support for `<<` merge syntax
- **Multiline Strings**: Some formatting may not be preserved exactly
- **Special Values**: Some YAML special values may be parsed as strings
- **YAML Tags**: Custom type tags may not be fully supported

For detailed information about parser limitations, see the `../limitation/` folder.

## Learning Path

**Recommended order for learning**:
1. `basic_config` - Start with fundamentals
2. `nested_structures` - Learn navigation
3. `arrays_sequences` - Understand sequences
4. `data_types` - Master type handling
5. `app_config` - See real-world usage
6. `nested_maps` - Advanced navigation
7. `complex_data` - Complex structures
8. `nested_arrays` - Multi-dimensional data
9. `multiline_strings` - Text handling
10. `anchors_merge` - Advanced features (with limitations)

## Additional Resources

- **Main Project**: See [README.md](../README.md) for general library documentation
- **Limitations**: See [limitation](../limitation/) for detailed limitation testing
- **API Reference**: See header files in [include](../include/)
- **Tests**: See [tests](../tests/) for unit tests and validation

## Contributing

If you create additional usage examples or improvements:
1. Follow the existing naming conventions
2. Include both C++ and YAML files
3. Add comprehensive comments
4. Update this README with new examples
5. Test thoroughly with the build system

---

These examples provide a foundation for using the YAML Parser library effectively in your C++ projects. Each example is self-contained and can serve as a starting point for your own YAML parsing needs.
