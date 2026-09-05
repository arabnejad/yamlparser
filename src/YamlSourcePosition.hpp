#pragma once

#include <cstddef>

namespace yamlparser {
namespace internal {

// Identifies one character in the original YAML document.
struct SourcePosition {
  std::size_t lineNumber;
  std::size_t columnNumber;
};

} // namespace internal
} // namespace yamlparser
