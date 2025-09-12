\page sample_usage Sample Usage Examples
\tableofcontents

# Sample Usage

This folder contains runnable examples that demonstrate how to use **YamlParserLib**.

## Build & Run (from repo root)

```bash
cmake -S . -B build
cmake --build build --target sample_usage
cmake --build build --target run_all_sample_usage
```

## Running a single example

Executables are placed in `sample_usage/bin/`. Examples expect to be run with the current working directory set to **`sample_usage/`** so that relative paths like `yaml_files/...` resolve correctly.

```bash
cd sample_usage
./bin/basic_config
./bin/nested_structures
# ...
```

## Examples included

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
