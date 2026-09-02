#include "YamlException.hpp"
#include "YamlParser.hpp"

#include <iostream>

using namespace yamlparser;

int main() {
  try {
    const YamlValue  document     = YamlParser().parseFile("yaml_files/utf8_text.yaml");
    const YamlValue &utf8Examples = document.at("utf8_examples");

    std::cout << "Currency: " << utf8Examples.at("currency").asString() << '\n';
    std::cout << "Language: " << utf8Examples.at("language").asString() << '\n';
    std::cout << "Status: " << utf8Examples.at("status").asString() << '\n';
    std::cout << "Emoji: " << utf8Examples.at("emoji").asString() << '\n';
  } catch (const YamlException &error) {
    std::cerr << "Unable to parse UTF-8 examples: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
