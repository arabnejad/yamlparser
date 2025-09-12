# YamlParserLib

<img src="logo.png" alt="YamlParserLib Logo" width="20%">

A modern C++14 YAML parser library with a tidy CMake build, GoogleTest suite, optional formatting and coverage tooling.

## Table of Contents
- [Features](#features)
- [Requirements](#requirements)
- [Build, Test, Format & Coverage](#build-test-format--coverage)
- [Usage](#usage)
- [Sample Usage Examples](#sample-usage-examples)
- [Limitations](#limitations)
- [Contributing](#contributing)
- [License](#license)

## Features
- C++14 compatible (no third-party dependencies for YAML parsing)
- Core YAML 1.2 features:
  - Scalars (string, integer, float, boolean)
  - Sequences and mappings
  - Multiline strings (literal and folded)
  - Anchors and aliases
  - Merge keys (see limitations)
- Memory safety and exceptions:
  - RAII design
  - Smart pointer management where appropriate
  - Clear, exception-based error handling
- Developer workflow:
  - GoogleTest-based unit tests
  - Optional code coverage (gcovr)
  - clang-format integration
  - Modern CMake targets for lib, examples, and tests

## Requirements
- C++14 compiler (GCC 5+, Clang 3.4+, or MSVC 2017+)
- CMake 3.14+
- Optional:
  - `clang-format` (formatting)
  - `gcovr` (coverage)


## Build, Test, Format & Coverage

### Configure
```bash
cmake -S . -B build
```

### Commands

| Action                     | Command                                             |
|---------------------------|-----------------------------------------------------|
| **Clean all build trees** | `cmake --build build --target clean_all`            |
| **Build library**         | `cmake --build build --target yamlparser`           |
| **Build examples**        | `cmake --build build --target sample_usage`         |
| **Run all examples**      | `cmake --build build --target run_all_sample_usage` |
| **Build limitation ex.**  | `cmake --build build --target sample_limitation`    |
| **Run limitation ex.**    | `cmake --build build --target run_sample_limitation`|
| **Format code**           | `cmake --build build --target clang_format`         |
| **Build tests**           | `cmake --build build --target tests`                |
| **Run tests**             | `cmake --build build --target run_tests`            |
| **Run tests (via ctest)** | `cmake --build build --target run_tests_ctest`      |
| **Coverage (console)**    | `cmake --build build --target gcovr_console`        |
| **Coverage (HTML)**       | `cmake --build build --target gcovr_html`           |

> Tip: parallel builds: append `-- -j$(nproc)` (or `-j4`) after any `cmake --build` command.

### Formatting
```bash
cmake --build build --target clang_format
```
Applies `.clang-format` to `src`, `include`, `tests`, `limitation/sample_test`, and `sample_usage/src`.

### Coverage
```bash
# configure with coverage flags enabled
cmake -S . -B build -DENABLE_COVERAGE=ON
cmake --build build --target run_tests
cmake --build build --target gcovr_html
# HTML report at: build/coverage/index.html

# or console summary
cmake --build build --target gcovr_console
```

## Usage

### CMake Integration

Use one of these approaches:

**1) FetchContent (recommended)**
```cmake
include(FetchContent)
FetchContent_Declare(
  yamlparser
  GIT_REPOSITORY https://github.com/arabnejad/yamlparser.git
  GIT_TAG main
)
FetchContent_MakeAvailable(yamlparser)
target_link_libraries(your_target PRIVATE yamlparser)
```

**2) add_subdirectory (e.g., git submodule)**
```cmake
add_subdirectory(third_party/yamlparser)
target_link_libraries(your_target PRIVATE yamlparser)
```

**3) Manual integration**
```cmake
add_library(yamlparser STATIC
  yamlparser/src/YamlParser.cpp
  yamlparser/src/YamlElement.cpp
  yamlparser/src/YamlHelperFunctions.cpp
  yamlparser/src/YamlPrinter.cpp
)
target_include_directories(yamlparser PUBLIC yamlparser/include)
target_link_libraries(your_target PRIVATE yamlparser)
```

### Error Handling

The library uses **exception-based error handling** for robust error management. All exceptions derive from `yamlparser::YamlException` and provide clear diagnostics.

```cpp
#include "YamlParser.hpp"
#include "YamlException.hpp"
using namespace yamlparser;

int main() {
  try {
    YamlParser parser;
    parser.parse("config.yaml"); // throws FileException if not found, SyntaxException on parse errors

   if (parser.isSequenceRoot()) {
      const auto& seq = parser.sequenceRoot();
      const auto& item = YamlElement::at(seq, 0); // IndexException on out of bounds
      std::cout << item.value.asString() << "\n"; // TypeException if not string
    } else {
      const auto& cfg  = parser.root();
      const auto& name = YamlElement::at(cfg, "name"); // KeyException if key missing
      std::cout << name.value.asString() << "\n"; // throws TypeException if not string
    }
  } catch (const YamlException& e) {
    std::cerr << "YAML error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
```

Exception types include: `FileException`, `SyntaxException`, `TypeException`, `KeyException`, `IndexException`, `ConversionException`, `StructureException`.

## Sample Usage Examples

See **[sample_usage/](./sample_usage/README.md)** for 10+ complete examples with YAML files.

**Quick start**
```bash
# build & run all
cmake --build build --target sample_usage
cmake --build build --target run_all_sample_usage

# run one (from the sample_usage folder)
cd sample_usage
./bin/basic_config
./bin/nested_structures
# ...
```

> Examples expect to run with the working directory set to `sample_usage/` so that relative paths like `yaml_files/...` resolve.

## Limitations

A few YAML constructs are intentionally simplified or restricted:

- **Merge keys with inline comments** – inline comments can break merges
- **Nested sequences** – sequences within sequences may turn into empty maps
- **String escape sequences** – escapes are treated literally (not processed)
- **Boolean recognition** – only lowercase `true`/`false` recognized
- **Empty values** – treated as empty strings rather than nulls

To explore and run the limitation samples:
```bash
# from the repo root
cmake --build build --target sample_limitation
cmake --build build --target run_sample_limitation
# executables are placed in limitation/bin; run working dir is limitation/
```

For background, see [limitation/limitation.md](./limitation/limitation.md).

## Contributing

Contributions are welcome! Please:
1. Follow the existing style (`cmake --build build --target clang_format`)
2. Add/update tests as needed (`tests/` + `run_tests`)
3. Update docs for new features
4. Keep or improve coverage when possible

**Workflow**
```bash
git checkout -b feature/amazing-feature
# hack hack
cmake --build build --target clang_format
git commit -m "feat: add amazing feature"
git push origin feature/amazing-feature
# open a PR
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
