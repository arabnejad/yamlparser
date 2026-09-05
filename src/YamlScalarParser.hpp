#pragma once

#include "YamlSourcePosition.hpp"
#include "YamlValue.hpp"

#include <string>

namespace yamlparser {
namespace internal {

// Removes quotes and decodes the supported escape characters.
// For example, `"line\\nnext"` becomes a string containing a real newline.
std::string parseQuotedScalar(const std::string &text, SourcePosition sourcePosition);

// Converts unquoted YAML text to the most suitable YamlValue type.
// For example, `true` becomes a boolean, `42` becomes an integer, and `host`
// remains a string.
YamlValue parsePlainScalarValue(const std::string &text);

} // namespace internal
} // namespace yamlparser
