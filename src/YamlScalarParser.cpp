#include "YamlScalarParser.hpp"

#include "YamlException.hpp"

#include <regex>

namespace yamlparser {
namespace internal {

namespace {

std::string trimWhitespace(const std::string &text) {
  const std::size_t first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    return "";
  const std::size_t last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1U);
}

} // namespace

std::string parseQuotedScalar(const std::string &text, SourcePosition sourcePosition) {
  if (text.empty() || (text.front() != '\'' && text.front() != '"'))
    throw SyntaxException("Expected a quoted string", sourcePosition.lineNumber, sourcePosition.columnNumber);

  const char  quote = text.front();
  std::string result;
  result.reserve(text.size() - 1U);

  for (std::size_t index = 1U; index < text.size(); ++index) {
    const char character = text[index];

    if (character == quote) {
      if (quote == '\'' && index + 1U < text.size() && text[index + 1U] == '\'') {
        result.push_back('\'');
        ++index;
        continue;
      }
      if (!trimWhitespace(text.substr(index + 1U)).empty())
        throw SyntaxException("Unexpected content after quoted string", sourcePosition.lineNumber,
                              sourcePosition.columnNumber + index + 1U);
      return result;
    }

    if (quote == '\'' || character != '\\') {
      result.push_back(character);
      continue;
    }

    if (index + 1U >= text.size())
      throw SyntaxException("Incomplete escape sequence", sourcePosition.lineNumber,
                            sourcePosition.columnNumber + index);

    const char escaped = text[++index];
    switch (escaped) {
    case 'b': // YAML \b: backspace character (U+0008).
      result.push_back('\b');
      break;
    case 't': // YAML \t: horizontal tab (U+0009).
      result.push_back('\t');
      break;
    case 'n': // YAML \n: line feed (U+000A).
      result.push_back('\n');
      break;
    case 'f': // YAML \f: form feed (U+000C).
      result.push_back('\f');
      break;
    case 'r': // YAML \r: carriage return (U+000D).
      result.push_back('\r');
      break;
    case '"': // YAML \": a literal double quote (U+0022).
      result.push_back('"');
      break;
    case '/': // YAML \/: a literal slash (U+002F).
      result.push_back('/');
      break;
    case '\\': // YAML \\: a literal backslash (U+005C).
      result.push_back('\\');
      break;
    default: // This parser supports only the common escapes listed above.
      throw SyntaxException(std::string("Unsupported escape sequence: \\") + escaped, sourcePosition.lineNumber,
                            sourcePosition.columnNumber + index - 1U);
    }
  }

  throw SyntaxException("Quoted strings must close on the same line", sourcePosition.lineNumber,
                        sourcePosition.columnNumber);
}

YamlValue parsePlainScalarValue(const std::string &text) {
  if (text == "null" || text == "~")
    return YamlValue();
  if (text == "true" || text == "True" || text == "TRUE")
    return YamlValue(true);
  if (text == "false" || text == "False" || text == "FALSE")
    return YamlValue(false);

  // static compiles each pattern once, and const prevents later changes.
  // ^ and $ require the entire value to match. [+-]? allows one optional
  // sign, and \d+ requires one or more decimal digits.
  static const std::regex integerPattern(R"(^[+-]?\d+$)");

  // (?:...) groups without capturing, and | separates these number forms:
  //   \d+\.\d*  digits followed by a decimal point, such as "12." or "12.5";
  //   \.\d+      a decimal point followed by digits, such as ".5";
  //   \d+         digits without a decimal point, such as "12".
  // (?:[eE][+-]?\d+)? adds an optional exponent such as "e3" or "E-2".
  // Plain integers also match this expression, but integerPattern handles
  // them first.
  static const std::regex floatingPointPattern(R"(^[+-]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][+-]?\d+)?$)");

  if (std::regex_match(text, integerPattern)) {
    try {
      return YamlValue(std::stoi(text));
    } catch (const std::exception &) {
      throw ConversionException(text, "integer");
    }
  }
  if (std::regex_match(text, floatingPointPattern)) {
    try {
      return YamlValue(std::stod(text));
    } catch (const std::exception &) {
      throw ConversionException(text, "double");
    }
  }
  return YamlValue(text);
}

} // namespace internal
} // namespace yamlparser
