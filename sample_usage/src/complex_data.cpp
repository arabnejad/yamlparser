#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <vector>

using namespace yamlparser;

void printValue(const YamlValue &value, int indent = 0) {
  const std::string indentStr(indent * 2, ' ');

  if (value.isString()) {
    std::cout << "\"" << value.asString() << "\"";
  } else if (value.isInteger()) {
    std::cout << value.asInteger();
  } else if (value.isDouble()) {
    std::cout << value.asDouble();
  } else if (value.isBoolean()) {
    std::cout << (value.asBoolean() ? "true" : "false");
  } else if (value.isMapping()) {
    std::cout << "{\n";
    const auto &map = value.asMapping();
    for (auto it = map.begin(); it != map.end(); ++it) {
      std::cout << indentStr << "  \"" << it->first << "\": ";
      printValue(it->second, indent + 1);
      if (std::next(it) != map.end())
        std::cout << ",";
      std::cout << "\n";
    }
    std::cout << indentStr << "}";
  } else if (value.isSequence()) {
    std::cout << "[\n";
    const auto &seq = value.asSequence();
    for (size_t i = 0; i < seq.size(); ++i) {
      std::cout << indentStr << "  ";
      printValue(seq[i], indent + 1);
      if (i < seq.size() - 1)
        std::cout << ",";
      std::cout << "\n";
    }
    std::cout << indentStr << "]";
  }
}

int main() {
  std::cout << "=== Complex Data Structures Parser Example ===\n\n";

  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/complex_data.yaml");
    const auto     &config   = document.asMapping();

    // Process users
    const auto &users = config.at("users").asSequence();

    std::cout << "Users:\n";
    std::cout << "------\n";
    for (size_t i = 0; i < users.size(); ++i) {
      if (users[i].isMapping()) {
        const auto &user = users[i].asMapping();
        std::cout << "User " << (i + 1) << ":\n";
        std::cout << "  ID: " << user.at("id").asInteger() << "\n";
        std::cout << "  Name: " << user.at("name").asString() << "\n";
        std::cout << "  Email: " << user.at("email").asString() << "\n";
        const auto &roles = user.at("roles").asSequence();
        std::cout << "  Roles: ";
        for (size_t j = 0; j < roles.size(); ++j) {
          std::cout << roles[j].asString();
          if (j < roles.size() - 1)
            std::cout << ", ";
        }
        std::cout << "\n";
        std::cout << "  Active: " << (user.at("active").asBoolean() ? "Yes" : "No") << "\n";
        std::cout << "  Last Login: " << user.at("last_login").asString() << "\n";
        std::cout << "\n";
      }
    }

    // Process groups
    const auto &groups = config.at("groups").asSequence();
    std::cout << "Groups:\n";
    std::cout << "-------\n";
    for (size_t i = 0; i < groups.size(); ++i) {
      const auto &group = groups[i].asMapping();
      std::cout << "Group " << (i + 1) << ":\n";
      std::cout << "  Name: " << group.at("name").asString() << "\n";

      const auto &perms = group.at("permissions").asSequence();
      std::cout << "  Permissions: ";
      for (size_t j = 0; j < perms.size(); ++j) {
        std::cout << perms[j].asString();
        if (j < perms.size() - 1)
          std::cout << ", ";
      }
      std::cout << "\n";

      const auto &members = group.at("members").asSequence();
      std::cout << "  Members: ";
      for (size_t j = 0; j < members.size(); ++j) {
        std::cout << members[j].asInteger();
        if (j < members.size() - 1)
          std::cout << ", ";
      }
      std::cout << "\n\n";
    }

    // Statistics
    std::cout << "Data Structure Statistics:\n";
    std::cout << "=========================\n";
    // Count users
    std::cout << "Total Users: " << config.at("users").asSequence().size() << "\n";
    // Count groups
    std::cout << "Total Groups: " << config.at("groups").asSequence().size() << "\n\n";

    std::cout << "This example demonstrates handling of nested user/group arrays.\n\n";
    std::cout << "\n✅ Successfully parsed complex user/group data structures!\n";
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
