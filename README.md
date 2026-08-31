# YamlParser

A small, dependency-free YAML parser for C++14 configuration files.

## Features

- Null, string, integer, double, and boolean values
- Block and inline sequences
- Nested sequences and mappings
- Literal (`|`) and folded (`>`) block strings
- Anchors, aliases, and mapping merge keys
- Mapping, sequence, and scalar document roots
- YAML printing with value-preserving parse/print round trips
- Exceptions with line numbers for syntax errors

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
make docs
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
| `YAMLPARSER_BUILD_DOCS` | Off | Enable the `docs` target |
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

Tests, examples, documentation, coverage, and sanitizers are disabled when the library is consumed as a dependency unless explicitly requested.

## Scope

This project implements a practical YAML subset rather than the complete YAML specification.

Current limitations:

- Boolean values are recognized only as lowercase `true` and `false`. Spellings such as `True`, `FALSE`, `yes`, and `no` remain strings.
- Double-quoted strings support `\n`, `\r`, `\t`, `\\`, `\"`, and `\uXXXX`, but not YAML's complete escape set.
- Tags and directives are not interpreted.
- Mapping keys must be scalar values; YAML complex keys are not supported.
- Flow sequences such as `[one, two]` are supported, but the only supported flow mapping is the empty mapping `{}`.
- A stream may contain only one YAML document. The optional `---` and `...` markers are supported for that document.
- A merge key may reference one mapping alias, such as `<<: *defaults`; merge lists such as `<<: [*first, *second]` are not supported.

Earlier versions also had problems with merge keys followed by inline comments, nested block sequences, escape processing, and distinguishing null values from empty strings. Those cases are supported and covered by the current test suite.

## License

MIT. See [LICENSE](LICENSE).
