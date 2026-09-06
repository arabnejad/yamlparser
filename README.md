# YamlParser

A small, dependency-free YAML parser for C++14 configuration files.

## Features

- Null, string, integer, double, and boolean values
- YAML core-schema boolean spellings
- Common double-quoted escapes: `\b`, `\t`, `\n`, `\f`, `\r`, `\"`, `\/`, and `\\`
- Literal UTF-8 text
- Block and flow sequences and mappings
- Nested sequences and mappings
- Literal (`|`) and folded (`>`) block strings
- Anchors and aliases
- Merge keys with one alias or a list of aliases
- Mapping, sequence, and scalar document roots
- YAML printing with value-preserving parse/print round trips
- Syntax errors with line numbers and flow-token column numbers

## Build and test

The root `Makefile` provides short commands for the complete workflow:

```bash
make help
make build
make test
make sanitizers
make examples
make run-examples
make format
make coverage
```

Useful variables can be overridden on the command line:

```bash
make test BUILD_TYPE=Release JOBS=8
make install INSTALL_PREFIX="$HOME/.local"
```

Direct CMake usage remains available:

```bash
cmake -S . -B build -DYAMLPARSER_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The main CMake options are:

| Option | Default | Purpose |
| --- | --- | --- |
| `YAMLPARSER_BUILD_TESTS` | On for the root project | Build tests |
| `YAMLPARSER_BUILD_EXAMPLES` | Off | Build examples |
| `YAMLPARSER_ENABLE_COVERAGE` | Off | Enable gcovr coverage |
| `YAMLPARSER_ENABLE_SANITIZERS` | Off | Enable ASan and UBSan |

## Usage

```cpp
#include "YamlParser.hpp"
#include "YamlPrinter.hpp"

#include <iostream>

using namespace yamlparser;

int main() {
  try {
    const YamlValue document = YamlParser().parseFile("config.yaml");

    const std::string &host = document.at("server").at("host").asString();
    const int port = document.at("server").at("port").asInteger();

    std::cout << host << ':' << port << '\n';
    YamlPrinter::print(document, std::cout);
  } catch (const YamlException &error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
```

Text and streams can be parsed without creating temporary files:

```cpp
const YamlParser parser;
YamlValue fromText = parser.parseText("enabled: true\n");
YamlValue fromStream = parser.parse(std::cin);
```

Flow sequences and mappings can be nested in either direction:

```yaml
server: {host: localhost, ports: [8080, 8443]}
workers: [{name: primary}, {name: backup}]
```

The printer emits these values in block style, so no formatting option is required to preserve their data or types.

Merge keys accept one mapping alias, a flow-style alias list, or a block-style
alias list:

```yaml
combined:
  <<: [*primary, *fallback]
  local_setting: true

another_combination:
  <<:
    - *primary
    - *fallback
```

Explicit keys override merged keys. When aliases in a merge list contain the
same key, the mapping named earlier in the list takes precedence.

A complete runnable example is available in
[`sample_usage/src/merge_lists.cpp`](sample_usage/src/merge_lists.cpp), with its
input in
[`sample_usage/yaml_files/merge_lists.yaml`](sample_usage/yaml_files/merge_lists.yaml).

`YamlValue` stores one of seven types:

```cpp
value.isNull();
value.isString();
value.isInteger();
value.isDouble();
value.isBoolean();
value.isSequence();
value.isMapping();
```

Use `at(index)` and `at(key)` to navigate sequences and mappings. Invalid types, keys, and indices throw exceptions derived from `YamlException`.

### UTF-8 text and quoted escapes

Write Unicode characters directly in UTF-8. `YamlValue` preserves their bytes in its `std::string` value:

```yaml
currency: "£"
language: "日本語"
status: "✓"
emoji: "😀"
```

Double-quoted strings support the common escapes `\b`, `\t`, `\n`, `\f`, `\r`, `\"`, `\/`, and `\\`. Numeric Unicode escapes such as `\u00A3` and `\U0001F600` produce a syntax error; write `£` and `😀` directly instead.

Double-quoted strings must close on the same line. Use literal (`|`) or folded (`>`) block strings for multiline text.

`YamlPrinter` throws `TypeException` when a string contains a control character that cannot be represented using the supported escapes.

## CMake integration

```cmake
include(FetchContent)
FetchContent_Declare(
  yamlparser
  GIT_REPOSITORY https://github.com/arabnejad/yamlparser.git
  GIT_TAG main
)
FetchContent_MakeAvailable(yamlparser)

target_link_libraries(your_target PRIVATE yamlparser::yamlparser)
```

Tests, examples, coverage, and sanitizers are disabled when the library is consumed as a dependency unless explicitly requested.

## Scope

This project implements a practical YAML subset rather than the complete YAML specification.

Current limitations:

- Numeric Unicode escapes (`\xXX`, `\uXXXX`, and `\UXXXXXXXX`) and YAML named escapes (`\N`, `\_`, `\L`, and `\P`) are not supported. Literal UTF-8 text is supported.
- The uncommon escapes `\0`, `\a`, `\v`, `\e`, and backslash-space are not supported.
- Double-quoted strings cannot span physical lines and do not support backslash line continuation. Use `|` or `>` block strings for multiline text.
- Tags and directives are not interpreted.
- Mapping keys must be scalar values; YAML complex keys are not supported.
- A flow collection must open and close on the same physical line. Multiline flow sequences and mappings are not supported.
- A stream may contain only one YAML document. The optional `---` and `...` markers are supported for that document.

Earlier versions also had problems with boolean spellings, merge keys followed by inline comments, nested block sequences, common escape processing, and distinguishing null values from empty strings. Those cases are supported and covered by the current test suite.

## License

MIT. See [LICENSE](LICENSE).
