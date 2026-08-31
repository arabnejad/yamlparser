#pragma once

#include "YamlValue.hpp"

#include <istream>
#include <string>

namespace yamlparser {

/** Parses the supported YAML subset into a YamlValue tree. */
class YamlParser {
public:
  YamlValue parseFile(const std::string &filename) const;
  YamlValue parse(std::istream &input) const;
  YamlValue parseText(const std::string &yamlText) const;
};

} // namespace yamlparser
