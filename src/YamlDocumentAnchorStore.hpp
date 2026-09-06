#pragma once

#include "YamlSourcePosition.hpp"
#include "YamlValue.hpp"

#include <map>
#include <string>
#include <vector>

namespace yamlparser {
namespace internal {

// Identifies one alias found in a merge value.
struct AliasReference {
  std::string    anchorName;
  SourcePosition sourcePosition;
};

// Owns the anchors defined while parsing one YAML document. DocumentParser
// creates a new store for every parse operation, so definitions cannot leak
// into later documents.
class DocumentAnchorStore {
public:
  // Defines an anchor by saving a copy of its parsed value.
  // For example, defineAnchor("defaults", value) registers `&defaults`.
  void defineAnchor(const std::string &anchorName, const YamlValue &value);

  // Returns the stored value referenced by an alias name without its `*`.
  // For example, resolveAlias("defaults") resolves `*defaults`.
  const YamlValue &resolveAlias(const std::string &aliasName) const;

  // Merges mappings from first to last. Existing target keys are not replaced,
  // so explicit keys and earlier aliases keep precedence.
  void mergeMappingsFromAliases(const std::vector<AliasReference> &aliases, YamlMapping &targetMapping) const;

private:
  std::map<std::string, YamlValue> m_anchorsByName;
};

} // namespace internal
} // namespace yamlparser
