# YAML Parser Limitation Tests

This folder contains a test suite for demonstrating the confirmed limitations of the C++ YAML parser library.

## Test Summary

✅ **6 Limitation Tests** - All demonstrating actual parser limitations:

1. **Merge Key Inline Comments** - Inline comments break merge functionality
2. **Nested Sequences** - Sequences within sequences become empty maps
3. **String Escape Sequences** - Escape sequences kept literal, not processed
4. **Boolean Recognition** - Only lowercase true/false recognized
6. **Empty Values/Nulls** - Empty values treated as empty strings

## Quick Start

```bash
# Build all tests
cmake .
make

# Run all limitation tests
./run_all_tests.sh
```

## Contents

- **limitation.md** - Detailed documentation of each limitation with workarounds
- **sample_yaml/** - YAML files that demonstrate parsing limitations
- **sample_test/** - C++ test programs that verify each limitation
- **CMakeLists.txt** - Build configuration (independent of main project)
- **run_all_tests.sh** - Script to run all tests sequentially

## What This Tests

Each test demonstrates the specific limitation and shows the actual parser behavior, confirming the documented limitations exist.
