#pragma once

#include "YamlValue.hpp"

#include <ostream>
#include <string>

namespace yamlparser {

/** Writes a YamlValue tree as YAML text. */
class YamlPrinter {
public:
  static void        print(const YamlValue &document, std::ostream &output);
  static std::string toString(const YamlValue &document);
};

} // namespace yamlparser
