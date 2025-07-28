
/**
 * \page limitation_doc Limitation Documentation
 * \tableofcontents
 */

## YAML Parser Confirmed Limitations

This document outlines the **confirmed limitations** of the C++ YAML parser library based on systematic testing.

## Merge Key Inline Comment Limitation
- **Issue**: Merge keys fail when followed by inline comments
- **Description**: While merge keys (<<: *anchor) work correctly, they fail to merge values when the merge key line contains an inline comment
- **Workaround**: Remove inline comments from merge key lines
- **Test**: `merge_comment_test.cpp` with `merge_comment_test.yaml`

## Nested Sequence Limitations
- **Issue**: Nested sequences (sequences containing other sequences) are not parsed correctly
- **Description**: The parser recognizes the outer sequence but interprets nested sequence items as empty maps instead of sequences
- **Workaround**: Avoid nested sequences; restructure as sequences of mappings or flatten the structure
- **Test**: `nested_seq_test.cpp` with `nested_seq_test.yaml`

## String Escape Sequences
- **Issue**: Escape sequences in quoted strings are kept literal, not processed
- **Description**: Strings containing escape sequences like `\t`, `\n` are parsed but escape sequences remain as literal text
- **Workaround**: Avoid escape sequences in strings, use simple quoted strings
- **Test**: `escape_test.cpp` with `escape_test.yaml`

## Boolean Value Recognition
- **Issue**: Only lowercase boolean values are recognized
- **Description**: The parser only recognizes `true` and `false` (lowercase) as boolean values. Mixed case variants are treated as strings
- **Workaround**: Use only lowercase `true` and `false` for boolean values
- **Test**: `boolean_test.cpp` with `boolean_test.yaml`

## Scientific Notation
- **Issue**: Scientific notation in numbers is parsed as strings
- **Description**: Numbers in scientific notation (e.g., `1.23e-4`) are treated as string values, not numeric values
- **Workaround**: Use standard decimal notation instead of scientific notation
- **Test**: `scientific_test.cpp` with `scientific_test.yaml`

## Empty Values and Implicit Nulls
- **Issue**: Empty values are treated as empty strings or "null" strings
- **Description**: Keys with empty values become empty strings rather than null values
- **Workaround**: Use explicit values when needed, be aware of empty string behavior
- **Test**: `null_test.cpp` with `null_test.yaml`

## How to Run Tests

1. Build the test executables:
   ```bash
   cd limitation
   cmake .
   make
   ```

2. Run all tests at once:
   ```bash
   ./run_all_tests.sh
   ```

3. Or run individual tests:
   ```bash
   ./sample_test/merge_comment_test
   ./sample_test/nested_seq_test
   ./sample_test/escape_test
   ./sample_test/boolean_test
   ./sample_test/scientific_test
   ./sample_test/null_test
   ```

Each test will demonstrate the specific limitation and show the actual parser behavior.

## Folder Structure

```
limitation/
├── limitation.md                # This documentation
├── CMakeLists.txt               # Build configuration
├── run_all_tests.sh             # Test runner script
├── sample_yaml/                 # YAML files demonstrating limitations
│   ├── merge_comment_test.yaml
│   ├── nested_seq_test.yaml
│   ├── escape_test.yaml
│   ├── boolean_test.yaml
│   ├── scientific_test.yaml
│   └── null_test.yaml
└── sample_test/                 # C++ test programs and executables
    ├── merge_comment_test.cpp   → merge_comment_test
    ├── nested_seq_test.cpp      → nested_seq_test
    ├── escape_test.cpp          → escape_test
    ├── boolean_test.cpp         → boolean_test
    ├── scientific_test.cpp      → scientific_test
    └── null_test.cpp            → null_test
```
