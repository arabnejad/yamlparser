#include "YamlException.hpp"
#include "YamlParser.hpp"

#include <iostream>
#include <string>

using namespace yamlparser;

namespace {

void printServiceSettings(const std::string &sectionTitle, const YamlValue &serviceSettings) {
  std::cout << sectionTitle << ":\n";
  std::cout << "  host: " << serviceSettings.at("host").asString() << '\n';
  std::cout << "  port: " << serviceSettings.at("port").asInteger() << '\n';
  std::cout << "  timeout: " << serviceSettings.at("timeout").asInteger() << '\n';
  std::cout << "  retries: " << serviceSettings.at("retries").asInteger() << '\n';
  std::cout << "  log_level: " << serviceSettings.at("log_level").asString() << "\n\n";
}

} // namespace

int main() {
  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/merge_lists.yaml");

    std::cout << "=== Multiple Mapping Alias Merge Example ===\n\n";
    printServiceSettings("Flow-style merge list", document.at("flow_style_service"));
    printServiceSettings("Block-style merge list", document.at("block_style_service"));

    std::cout << "The flow-style service keeps its explicit timeout and log level.\n";
    std::cout << "The block-style service gets duplicate values from primary_defaults,\n";
    std::cout << "because that alias appears before fallback_defaults.\n";
  } catch (const YamlException &error) {
    std::cerr << "Unable to parse the merge-list example: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
