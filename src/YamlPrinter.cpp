#include "YamlPrinter.hpp"

#include <cctype>
#include <iomanip>
#include <limits>
#include <regex>
#include <sstream>

namespace yamlparser {
namespace {

std::string indentation(std::size_t width) {
  return std::string(width, ' ');
}

bool requiresQuotes(const std::string &value) {
  if (value.empty() || value == "null" || value == "~" || value == "true" || value == "false")
    return true;

  static const std::regex numericPattern("^[+-]?(?:\\d+|(?:\\d+\\.\\d*|\\.\\d+|\\d+)(?:[eE][+-]?\\d+)?)$");
  if (std::regex_match(value, numericPattern))
    return true;

  if (std::isspace(static_cast<unsigned char>(value.front())) != 0 ||
      std::isspace(static_cast<unsigned char>(value.back())) != 0)
    return true;
  if (value.front() == '-' || value.front() == '?' || value.front() == ':' || value.front() == '&' ||
      value.front() == '*' || value.front() == '!' || value.front() == '|' || value.front() == '>' ||
      value.front() == '@' || value.front() == '`' || value.front() == '[' || value.front() == '{')
    return true;
  return value.find_first_of(":#{}[],\n\r\t\"'") != std::string::npos;
}

std::string quoteString(const std::string &value) {
  const bool containsControlCharacter = value.find_first_of("\n\r\t") != std::string::npos;
  if (!containsControlCharacter) {
    std::string quoted("'");
    for (char character : value) {
      quoted.push_back(character);
      if (character == '\'')
        quoted.push_back('\'');
    }
    quoted.push_back('\'');
    return quoted;
  }

  std::string quoted("\"");
  for (char character : value) {
    switch (character) {
    case '\n':
      quoted += "\\n";
      break;
    case '\r':
      quoted += "\\r";
      break;
    case '\t':
      quoted += "\\t";
      break;
    case '\\':
      quoted += "\\\\";
      break;
    case '"':
      quoted += "\\\"";
      break;
    default:
      quoted.push_back(character);
      break;
    }
  }
  quoted.push_back('"');
  return quoted;
}

std::string formatString(const std::string &value) {
  return requiresQuotes(value) ? quoteString(value) : value;
}

bool isEmptyContainer(const YamlValue &value) {
  return (value.isSequence() && value.asSequence().empty()) || (value.isMapping() && value.asMapping().empty());
}

bool isBlockValue(const YamlValue &value) {
  return (value.isSequence() || value.isMapping()) && !isEmptyContainer(value);
}

void writeScalarOrEmptyContainer(const YamlValue &value, std::ostream &output) {
  switch (value.type()) {
  case YamlValue::Type::Null:
    output << "null";
    break;
  case YamlValue::Type::String:
    output << formatString(value.asString());
    break;
  case YamlValue::Type::Double:
    output << std::setprecision(std::numeric_limits<double>::max_digits10) << value.asDouble();
    break;
  case YamlValue::Type::Integer:
    output << value.asInteger();
    break;
  case YamlValue::Type::Boolean:
    output << (value.asBoolean() ? "true" : "false");
    break;
  case YamlValue::Type::Sequence:
    output << "[]";
    break;
  case YamlValue::Type::Mapping:
    output << "{}";
    break;
  }
}

void writeValue(const YamlValue &value, std::ostream &output, std::size_t indentationWidth) {
  if (value.isMapping()) {
    if (value.asMapping().empty()) {
      output << indentation(indentationWidth) << "{}\n";
      return;
    }
    for (const auto &entry : value.asMapping()) {
      output << indentation(indentationWidth) << formatString(entry.first) << ':';
      if (isBlockValue(entry.second)) {
        output << '\n';
        writeValue(entry.second, output, indentationWidth + 2U);
      } else {
        output << ' ';
        writeScalarOrEmptyContainer(entry.second, output);
        output << '\n';
      }
    }
    return;
  }

  if (value.isSequence()) {
    if (value.asSequence().empty()) {
      output << indentation(indentationWidth) << "[]\n";
      return;
    }
    for (const YamlValue &item : value.asSequence()) {
      output << indentation(indentationWidth) << '-';
      if (isBlockValue(item)) {
        output << '\n';
        writeValue(item, output, indentationWidth + 2U);
      } else {
        output << ' ';
        writeScalarOrEmptyContainer(item, output);
        output << '\n';
      }
    }
    return;
  }

  output << indentation(indentationWidth);
  writeScalarOrEmptyContainer(value, output);
  output << '\n';
}

} // namespace

void YamlPrinter::print(const YamlValue &document, std::ostream &output) {
  writeValue(document, output, 0U);
}

std::string YamlPrinter::toString(const YamlValue &document) {
  std::ostringstream output;
  print(document, output);
  return output.str();
}

} // namespace yamlparser
