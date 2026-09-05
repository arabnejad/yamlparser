#pragma once

#include "YamlSourcePosition.hpp"
#include "YamlValue.hpp"

#include <map>
#include <string>

namespace yamlparser {
namespace internal {

// Parses one complete flow collection and returns its value.
// For example, `{values: [1, 2]}` becomes a mapping containing a sequence.
// sourcePosition identifies where the opening bracket or brace appears in the
// YAML document so syntax errors can point to the right place.
YamlValue parseFlowCollectionValue(const std::string &expression, SourcePosition sourcePosition,
                                   std::map<std::string, YamlValue> &anchors);

} // namespace internal
} // namespace yamlparser
