#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <vector>

using namespace yamlparser;

void printValue(const YamlElement &value, int indent = 0) {
  const std::string indentStr(indent * 2, ' ');

  if (value.isString()) {
    std::cout << "\"" << value.asString() << "\"";
  } else if (value.isInt()) {
    std::cout << value.asInt();
  } else if (value.isDouble()) {
    std::cout << value.asDouble();
  } else if (value.isBool()) {
    std::cout << (value.asBool() ? "true" : "false");
  } else if (value.isMap()) {
    std::cout << "{\n";
    const auto &map = value.asMap();
    for (auto it = map.begin(); it != map.end(); ++it) {
      std::cout << indentStr << "  \"" << it->first << "\": ";
      printValue(it->second.value, indent + 1);
      if (std::next(it) != map.end())
        std::cout << ",";
      std::cout << "\n";
    }
    std::cout << indentStr << "}";
  } else if (value.isSeq()) {
    std::cout << "[\n";
    const auto &seq = value.asSeq();
    for (size_t i = 0; i < seq.size(); ++i) {
      std::cout << indentStr << "  ";
      printValue(seq[i].value, indent + 1);
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
    YamlParser parser;
    parser.parse("yaml_files/complex_data.yaml");
    if (parser.isSequenceRoot()) {
      std::cerr << "Error: Expected root to be a map, but got sequence\n";
      return 1;
    }

    const auto &config = parser.root();

    // Process users
    const auto &users = config.at("users").value.asSeq();

    std::cout << "Users:\n";
    std::cout << "------\n";
    for (size_t i = 0; i < users.size(); ++i) {
      if (users[i].value.isMap()) {
        const auto &user = users[i].value.asMap();
        std::cout << "User " << (i + 1) << ":\n";
        std::cout << "  ID: " << user.at("id").value.asInt() << "\n";
        std::cout << "  Name: " << user.at("name").value.asString() << "\n";
        std::cout << "  Email: " << user.at("email").value.asString() << "\n";
        const auto &roles = user.at("roles").value.asSeq();
        std::cout << "  Roles: ";
        for (size_t j = 0; j < roles.size(); ++j) {
          std::cout << roles[j].value.asString();
          if (j < roles.size() - 1)
            std::cout << ", ";
        }
        std::cout << "\n";
        std::cout << "  Active: " << (user.at("active").value.asBool() ? "Yes" : "No") << "\n";
        std::cout << "  Last Login: " << user.at("last_login").value.asString() << "\n";
        std::cout << "\n";
      }
    }

    // Process groups
    const auto &groups = config.at("groups").value.asSeq();
    std::cout << "Groups:\n";
    std::cout << "-------\n";
    for (size_t i = 0; i < groups.size(); ++i) {
      const auto &group = groups[i].value.asMap();
      std::cout << "Group " << (i + 1) << ":\n";
      std::cout << "  Name: " << group.at("name").value.asString() << "\n";

      const auto &perms = group.at("permissions").value.asSeq();
      std::cout << "  Permissions: ";
      for (size_t j = 0; j < perms.size(); ++j) {
        std::cout << perms[j].value.asString();
        if (j < perms.size() - 1)
          std::cout << ", ";
      }
      std::cout << "\n";

      const auto &members = group.at("members").value.asSeq();
      std::cout << "  Members: ";
      for (size_t j = 0; j < members.size(); ++j) {
        std::cout << members[j].value.asInt();
        if (j < members.size() - 1)
          std::cout << ", ";
      }
      std::cout << "\n\n";
    }

    // Statistics
    std::cout << "Data Structure Statistics:\n";
    std::cout << "=========================\n";
    // Count users
    std::cout << "Total Users: " << config.at("users").value.asSeq().size() << "\n";
    // Count groups
    std::cout << "Total Groups: " << config.at("groups").value.asSeq().size() << "\n\n";

    std::cout << "This example demonstrates handling of nested user/group arrays.\n\n";
    std::cout << "\n✅ Successfully parsed complex user/group data structures!\n";
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
