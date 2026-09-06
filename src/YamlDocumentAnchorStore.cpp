#include "YamlDocumentAnchorStore.hpp"

#include "YamlException.hpp"

namespace yamlparser {
namespace internal {

void DocumentAnchorStore::defineAnchor(const std::string &anchorName, const YamlValue &value) {
  m_anchorsByName[anchorName] = value;
}

const YamlValue &DocumentAnchorStore::resolveAlias(const std::string &aliasName) const {
  const auto anchorEntry = m_anchorsByName.find(aliasName);
  if (anchorEntry == m_anchorsByName.end())
    throw KeyException("*" + aliasName);
  return anchorEntry->second;
}

void DocumentAnchorStore::mergeMappingsFromAliases(const std::vector<AliasReference> &aliases,
                                                   YamlMapping                       &targetMapping) const {
  for (const AliasReference &alias : aliases) {
    const YamlValue &aliasedValue = resolveAlias(alias.anchorName);
    if (!aliasedValue.isMapping())
      throw TypeException("Merge alias '*" + alias.anchorName + "' at line " +
                          std::to_string(alias.sourcePosition.lineNumber) + ", column " +
                          std::to_string(alias.sourcePosition.columnNumber) + " must refer to a mapping");

    const YamlMapping &sourceMapping = aliasedValue.asMapping();
    for (const auto &entry : sourceMapping)
      targetMapping.insert(entry);
  }
}

} // namespace internal
} // namespace yamlparser
