#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <string>

using namespace yamlparser;

void displayString(const std::string &label, const std::string &value) {
  std::cout << label << ":\n";
  std::cout << std::string(label.length(), '=') << "\n";
  std::cout << "Raw value: \"" << value << "\"\n";
  std::cout << "Length: " << value.length() << " characters\n";

  // Show special characters
  std::cout << "Special chars: ";
  for (char c : value) {
    if (c == '\n')
      std::cout << "\\n ";
    else if (c == '\t')
      std::cout << "\\t ";
    else if (c == '\r')
      std::cout << "\\r ";
    else if (c == ' ')
      std::cout << "· ";
    else
      std::cout << c;
  }
  std::cout << "\n\n";
}

int main() {
  std::cout << "=== Multiline Strings Parser Example ===\n\n";

  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/multiline_strings.yaml");
    const auto     &config   = document.asMapping();

    // description (literal block)
    displayString("Description (|)", config.at("description").asString());

    // folded_description (folded block)
    displayString("Folded Description (>)", config.at("folded_description").asString());

    // inline_string
    displayString("Inline String (quoted)", config.at("inline_string").asString());

    // unquoted_string
    displayString("Unquoted String", config.at("unquoted_string").asString());

    // multiline_message
    displayString("Multiline Message (|)", config.at("multiline_message").asString());

    std::cout << "✅ Successfully parsed multiline strings!\n";
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
