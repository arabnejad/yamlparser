#pragma once

#include "YamlDocumentAnchorStore.hpp"
#include "YamlSourcePosition.hpp"
#include "YamlValue.hpp"

#include <string>
#include <vector>

namespace yamlparser {
namespace internal {

// Parses one complete flow collection and returns its value.
// For example, `{values: [1, 2]}` becomes a mapping containing a sequence.
// sourcePosition identifies where the opening bracket or brace appears in the
// YAML document so syntax errors can point to the right place.
YamlValue parseFlowCollectionValue(const std::string &expression, SourcePosition sourcePosition,
                                   DocumentAnchorStore &anchorStore);

// Parses a complete single-line merge value and returns its alias references.
// Both `*one` and `[*one, *two]` are accepted.
std::vector<AliasReference> parseFlowMergeAliases(const std::string &expression, SourcePosition sourcePosition,
                                                  DocumentAnchorStore &anchorStore);

} // namespace internal
} // namespace yamlparser
