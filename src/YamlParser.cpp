#include "YamlParser.hpp"

#include <cctype>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

namespace yamlparser {
namespace {

std::string trimWhitespace(const std::string &text) {
  const std::size_t first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    return "";
  const std::size_t last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1U);
}

std::size_t indentationOf(const std::string &line) {
  const std::size_t firstContent = line.find_first_not_of(" \t");
  return firstContent == std::string::npos ? line.size() : firstContent;
}

std::string contentOf(const std::string &line) {
  return line.substr(indentationOf(line));
}

bool isSequenceMarker(const std::string &text) {
  return text == "-" ||
         (text.size() > 1U && text.front() == '-' && std::isspace(static_cast<unsigned char>(text[1])) != 0);
}

std::string removeInlineComment(const std::string &text) {
  bool insideSingleQuotes = false;
  bool insideDoubleQuotes = false;
  bool escapedCharacter   = false;

  for (std::size_t index = 0; index < text.size(); ++index) {
    const char character = text[index];
    if (insideDoubleQuotes && escapedCharacter) {
      escapedCharacter = false;
      continue;
    }
    if (insideDoubleQuotes && character == '\\') {
      escapedCharacter = true;
      continue;
    }
    if (character == '\'' && !insideDoubleQuotes) {
      insideSingleQuotes = !insideSingleQuotes;
      continue;
    }
    if (character == '"' && !insideSingleQuotes) {
      insideDoubleQuotes = !insideDoubleQuotes;
      continue;
    }
    const bool startsComment = character == '#' && !insideSingleQuotes && !insideDoubleQuotes &&
                               (index == 0U || std::isspace(static_cast<unsigned char>(text[index - 1U])) != 0);
    if (startsComment)
      return trimWhitespace(text.substr(0, index));
  }
  return trimWhitespace(text);
}

std::size_t findMappingSeparator(const std::string &text) {
  bool insideSingleQuotes = false;
  bool insideDoubleQuotes = false;
  bool escapedCharacter   = false;
  int  squareBracketDepth = 0;

  for (std::size_t index = 0; index < text.size(); ++index) {
    const char character = text[index];
    if (insideDoubleQuotes && escapedCharacter) {
      escapedCharacter = false;
      continue;
    }
    if (insideDoubleQuotes && character == '\\') {
      escapedCharacter = true;
      continue;
    }
    if (character == '\'' && !insideDoubleQuotes) {
      insideSingleQuotes = !insideSingleQuotes;
      continue;
    }
    if (character == '"' && !insideSingleQuotes) {
      insideDoubleQuotes = !insideDoubleQuotes;
      continue;
    }
    if (insideSingleQuotes || insideDoubleQuotes)
      continue;
    if (character == '[') {
      ++squareBracketDepth;
      continue;
    }
    if (character == ']') {
      --squareBracketDepth;
      continue;
    }
    if (character == ':' && squareBracketDepth == 0) {
      const bool hasValidSeparator =
          index + 1U == text.size() || std::isspace(static_cast<unsigned char>(text[index + 1U])) != 0;
      if (hasValidSeparator)
        return index;
    }
  }
  return std::string::npos;
}

std::string parseQuotedString(const std::string &text, std::size_t lineNumber) {
  if (text.empty() || (text.front() != '\'' && text.front() != '"'))
    throw SyntaxException("Expected a quoted string", lineNumber);

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
        throw SyntaxException("Unexpected content after quoted string", lineNumber);
      return result;
    }

    if (quote == '\'' || character != '\\') {
      result.push_back(character);
      continue;
    }

    if (index + 1U >= text.size())
      throw SyntaxException("Incomplete escape sequence", lineNumber);

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
      throw SyntaxException(std::string("Unsupported escape sequence: \\") + escaped, lineNumber);
    }
  }

  throw SyntaxException("Quoted strings must close on the same line", lineNumber);
}

YamlValue resolvePlainScalar(const std::string &value) {
  if (value == "null" || value == "~")
    return YamlValue();
  if (value == "true" || value == "True" || value == "TRUE")
    return YamlValue(true);
  if (value == "false" || value == "False" || value == "FALSE")
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

  if (std::regex_match(value, integerPattern)) {
    try {
      return YamlValue(std::stoi(value));
    } catch (const std::exception &) {
      throw ConversionException(value, "integer");
    }
  }
  if (std::regex_match(value, floatingPointPattern)) {
    try {
      return YamlValue(std::stod(value));
    } catch (const std::exception &) {
      throw ConversionException(value, "double");
    }
  }
  return YamlValue(value);
}

class DocumentParser {
public:
  explicit DocumentParser(std::vector<std::string> lines) : m_lines(std::move(lines)) {}

  YamlValue parseDocument() {
    skipIgnoredLines();
    if (atEnd())
      return YamlValue(YamlMapping());

    if (trimWhitespace(m_lines[m_nextLine]) == "---") {
      ++m_nextLine;
      skipIgnoredLines();
    }
    if (atEnd())
      return YamlValue(YamlMapping());

    const std::size_t rootIndentation = indentationOf(m_lines[m_nextLine]);
    YamlValue         document        = parseBlock(rootIndentation);

    skipIgnoredLines();
    if (!atEnd() && trimWhitespace(m_lines[m_nextLine]) == "...") {
      ++m_nextLine;
      skipIgnoredLines();
    }
    if (!atEnd())
      throw SyntaxException("Unexpected content after the root value", currentLineNumber());
    return document;
  }

private:
  std::vector<std::string>         m_lines;
  std::size_t                      m_nextLine = 0U;
  std::map<std::string, YamlValue> m_anchors;

  bool atEnd() const noexcept {
    return m_nextLine >= m_lines.size();
  }
  std::size_t currentLineNumber() const noexcept {
    return m_nextLine + 1U;
  }

  bool lineIsIgnored(std::size_t lineIndex) const {
    const std::string content = trimWhitespace(m_lines[lineIndex]);
    return content.empty() || content.front() == '#';
  }

  void skipIgnoredLines() {
    while (!atEnd() && lineIsIgnored(m_nextLine))
      ++m_nextLine;
  }

  YamlValue parseBlock(std::size_t expectedIndentation) {
    skipIgnoredLines();
    if (atEnd())
      return YamlValue();

    const std::size_t actualIndentation = indentationOf(m_lines[m_nextLine]);
    if (actualIndentation != expectedIndentation)
      throw SyntaxException("Unexpected indentation", currentLineNumber());

    const std::string content = removeInlineComment(contentOf(m_lines[m_nextLine]));
    if (content == "{}") {
      ++m_nextLine;
      return YamlValue(YamlMapping());
    }
    if (content == "[]") {
      ++m_nextLine;
      return YamlValue(YamlSequence());
    }
    if (isSequenceMarker(content))
      return YamlValue(parseSequence(expectedIndentation));
    if (findMappingSeparator(content) != std::string::npos)
      return YamlValue(parseMapping(expectedIndentation));

    const std::size_t lineNumber = currentLineNumber();
    ++m_nextLine;
    return parseValueOrNestedBlock(content, expectedIndentation, lineNumber);
  }

  YamlMapping parseMapping(std::size_t expectedIndentation) {
    YamlMapping           mapping;
    std::set<std::string> explicitlyDefinedKeys;
    parseRemainingMappingEntries(expectedIndentation, mapping, explicitlyDefinedKeys);
    return mapping;
  }

  YamlMapping parseMappingWithFirstEntry(const std::string &firstEntry, std::size_t mappingIndentation,
                                         std::size_t lineNumber) {
    YamlMapping           mapping;
    std::set<std::string> explicitlyDefinedKeys;
    parseMappingEntry(firstEntry, mappingIndentation, lineNumber, mapping, explicitlyDefinedKeys);
    parseRemainingMappingEntries(mappingIndentation, mapping, explicitlyDefinedKeys);
    return mapping;
  }

  void parseRemainingMappingEntries(std::size_t expectedIndentation, YamlMapping &mapping,
                                    std::set<std::string> &explicitlyDefinedKeys) {
    while (true) {
      skipIgnoredLines();
      if (atEnd())
        return;

      const std::size_t actualIndentation = indentationOf(m_lines[m_nextLine]);
      if (actualIndentation < expectedIndentation)
        return;
      if (actualIndentation > expectedIndentation)
        throw SyntaxException("Unexpected indentation in mapping", currentLineNumber());

      const std::string entry = removeInlineComment(contentOf(m_lines[m_nextLine]));
      if (isSequenceMarker(entry))
        return;

      const std::size_t lineNumber = currentLineNumber();
      ++m_nextLine;
      parseMappingEntry(entry, expectedIndentation, lineNumber, mapping, explicitlyDefinedKeys);
    }
  }

  void parseMappingEntry(const std::string &entry, std::size_t entryIndentation, std::size_t lineNumber,
                         YamlMapping &mapping, std::set<std::string> &explicitlyDefinedKeys) {
    const std::size_t separator = findMappingSeparator(entry);
    if (separator == std::string::npos)
      throw SyntaxException("Expected a 'key: value' mapping entry", lineNumber);

    const std::string rawKey = trimWhitespace(entry.substr(0, separator));
    if (rawKey.empty())
      throw SyntaxException("Mapping key cannot be empty", lineNumber);

    const std::string key =
        (rawKey.front() == '\'' || rawKey.front() == '"') ? parseQuotedString(rawKey, lineNumber) : rawKey;
    const std::string valueText = removeInlineComment(entry.substr(separator + 1U));

    if (key == "<<") {
      mergeAnchoredMapping(valueText, mapping, lineNumber);
      return;
    }
    if (!explicitlyDefinedKeys.insert(key).second)
      throw SyntaxException("Duplicate mapping key: '" + key + "'", lineNumber);

    mapping[key] = parseValueOrNestedBlock(valueText, entryIndentation, lineNumber);
  }

  YamlSequence parseSequence(std::size_t expectedIndentation) {
    YamlSequence sequence;
    parseRemainingSequenceItems(expectedIndentation, sequence);
    return sequence;
  }

  YamlSequence parseSequenceWithFirstItem(const std::string &firstItem, std::size_t sequenceIndentation,
                                          std::size_t lineNumber) {
    YamlSequence sequence;
    sequence.push_back(parseSequenceItem(firstItem, sequenceIndentation, lineNumber));
    parseRemainingSequenceItems(sequenceIndentation, sequence);
    return sequence;
  }

  void parseRemainingSequenceItems(std::size_t expectedIndentation, YamlSequence &sequence) {
    while (true) {
      skipIgnoredLines();
      if (atEnd())
        return;

      const std::size_t actualIndentation = indentationOf(m_lines[m_nextLine]);
      if (actualIndentation < expectedIndentation)
        return;
      if (actualIndentation > expectedIndentation)
        throw SyntaxException("Unexpected indentation in sequence", currentLineNumber());

      const std::string content = removeInlineComment(contentOf(m_lines[m_nextLine]));
      if (!isSequenceMarker(content))
        return;

      const std::size_t lineNumber = currentLineNumber();
      const std::string itemText   = trimWhitespace(content.substr(1U));
      ++m_nextLine;
      sequence.push_back(parseSequenceItem(itemText, expectedIndentation, lineNumber));
    }
  }

  YamlValue parseSequenceItem(const std::string &itemText, std::size_t sequenceIndentation, std::size_t lineNumber) {
    if (itemText.empty())
      return parseNestedBlockOrNull(sequenceIndentation);

    if (isSequenceMarker(itemText)) {
      const std::string firstNestedItem = trimWhitespace(itemText.substr(1U));
      return YamlValue(parseSequenceWithFirstItem(firstNestedItem, sequenceIndentation + 2U, lineNumber));
    }

    if (findMappingSeparator(itemText) != std::string::npos)
      return YamlValue(parseMappingWithFirstEntry(itemText, sequenceIndentation + 2U, lineNumber));

    return parseValueOrNestedBlock(itemText, sequenceIndentation, lineNumber);
  }

  YamlValue parseValueOrNestedBlock(const std::string &valueText, std::size_t parentIndentation,
                                    std::size_t lineNumber) {
    const std::string value = trimWhitespace(valueText);
    if (value.empty())
      return parseNestedBlockOrNull(parentIndentation);
    if (value.front() == '&')
      return parseAnchoredValue(value, parentIndentation, lineNumber);
    if (value.front() == '*')
      return resolveAlias(value, lineNumber);
    if (value.front() == '|' || value.front() == '>')
      return parseBlockScalar(value, parentIndentation);
    return parseInlineValue(value, lineNumber);
  }

  YamlValue parseNestedBlockOrNull(std::size_t parentIndentation) {
    skipIgnoredLines();
    if (atEnd() || indentationOf(m_lines[m_nextLine]) <= parentIndentation)
      return YamlValue();
    return parseBlock(indentationOf(m_lines[m_nextLine]));
  }

  YamlValue parseAnchoredValue(const std::string &anchorExpression, std::size_t parentIndentation,
                               std::size_t lineNumber) {
    std::size_t nameEnd = 1U;
    while (nameEnd < anchorExpression.size() &&
           std::isspace(static_cast<unsigned char>(anchorExpression[nameEnd])) == 0)
      ++nameEnd;

    const std::string anchorName = anchorExpression.substr(1U, nameEnd - 1U);
    if (anchorName.empty())
      throw SyntaxException("Anchor name cannot be empty", lineNumber);

    const std::string anchoredText  = trimWhitespace(anchorExpression.substr(nameEnd));
    YamlValue         anchoredValue = parseValueOrNestedBlock(anchoredText, parentIndentation, lineNumber);
    m_anchors[anchorName]           = anchoredValue;
    return anchoredValue;
  }

  YamlValue resolveAlias(const std::string &aliasExpression, std::size_t lineNumber) const {
    const std::string aliasName = trimWhitespace(aliasExpression.substr(1U));
    if (aliasName.empty() || aliasName.find_first_of(" \t") != std::string::npos)
      throw SyntaxException("Invalid alias", lineNumber);

    const auto anchor = m_anchors.find(aliasName);
    if (anchor == m_anchors.end())
      throw KeyException("*" + aliasName);
    return anchor->second;
  }

  void mergeAnchoredMapping(const std::string &aliasExpression, YamlMapping &targetMapping,
                            std::size_t lineNumber) const {
    const YamlValue source = resolveAlias(aliasExpression, lineNumber);
    if (!source.isMapping())
      throw TypeException("Merge alias must refer to a mapping");

    for (const auto &entry : source.asMapping())
      targetMapping.insert(entry);
  }

  YamlValue parseInlineValue(const std::string &value, std::size_t lineNumber) {
    if (value.empty())
      return YamlValue();
    if (value.front() == '\'' || value.front() == '"')
      return YamlValue(parseQuotedString(value, lineNumber));
    if (value == "[]")
      return YamlValue(YamlSequence());
    if (value == "{}")
      return YamlValue(YamlMapping());
    if (value.front() == '[') {
      if (value.back() != ']')
        throw SyntaxException("Inline sequence is missing its closing bracket", lineNumber);
      return YamlValue(parseFlowSequence(value, lineNumber));
    }
    return resolvePlainScalar(value);
  }

  YamlSequence parseFlowSequence(const std::string &expression, std::size_t lineNumber) {
    const std::string body = expression.substr(1U, expression.size() - 2U);
    if (trimWhitespace(body).empty())
      return YamlSequence();

    YamlSequence sequence;
    std::string  currentItem;
    bool         insideSingleQuotes = false;
    bool         insideDoubleQuotes = false;
    bool         escapedCharacter   = false;
    int          nestedDepth        = 0;

    for (char character : body) {
      if (insideDoubleQuotes && escapedCharacter) {
        currentItem.push_back(character);
        escapedCharacter = false;
        continue;
      }
      if (insideDoubleQuotes && character == '\\') {
        currentItem.push_back(character);
        escapedCharacter = true;
        continue;
      }
      if (character == '\'' && !insideDoubleQuotes)
        insideSingleQuotes = !insideSingleQuotes;
      else if (character == '"' && !insideSingleQuotes)
        insideDoubleQuotes = !insideDoubleQuotes;
      else if (!insideSingleQuotes && !insideDoubleQuotes && character == '[')
        ++nestedDepth;
      else if (!insideSingleQuotes && !insideDoubleQuotes && character == ']')
        --nestedDepth;

      if (character == ',' && !insideSingleQuotes && !insideDoubleQuotes && nestedDepth == 0) {
        appendFlowSequenceItem(currentItem, lineNumber, sequence);
        currentItem.clear();
      } else {
        currentItem.push_back(character);
      }
    }

    if (insideSingleQuotes || insideDoubleQuotes || nestedDepth != 0)
      throw SyntaxException("Malformed inline sequence", lineNumber);
    appendFlowSequenceItem(currentItem, lineNumber, sequence);
    return sequence;
  }

  void appendFlowSequenceItem(const std::string &itemText, std::size_t lineNumber, YamlSequence &sequence) {
    const std::string item = trimWhitespace(itemText);
    if (item.empty())
      throw SyntaxException("Inline sequence contains an empty item", lineNumber);
    sequence.push_back(parseInlineValue(item, lineNumber));
  }

  YamlValue parseBlockScalar(const std::string &header, std::size_t parentIndentation) {
    const char style          = header.front();
    const char chompingMethod = header.size() > 1U ? header[1] : '\0';

    struct ScalarLine {
      std::string text;
      bool        moreIndented;
    };

    std::vector<ScalarLine> scalarLines;
    std::size_t             contentIndentation = std::string::npos;

    while (!atEnd()) {
      const std::string &line = m_lines[m_nextLine];
      if (trimWhitespace(line).empty()) {
        scalarLines.push_back(ScalarLine{"", false});
        ++m_nextLine;
        continue;
      }

      const std::size_t lineIndentation = indentationOf(line);
      if (lineIndentation <= parentIndentation)
        break;
      if (contentIndentation == std::string::npos)
        contentIndentation = lineIndentation;

      const std::size_t textStart = line.size() < contentIndentation ? line.size() : contentIndentation;
      scalarLines.push_back(ScalarLine{line.substr(textStart), lineIndentation > contentIndentation});
      ++m_nextLine;
    }

    std::string result;
    if (style == '|') {
      for (const ScalarLine &line : scalarLines) {
        result += line.text;
        result.push_back('\n');
      }
    } else {
      for (std::size_t index = 0; index < scalarLines.size(); ++index) {
        if (index > 0U) {
          const ScalarLine &previous = scalarLines[index - 1U];
          const ScalarLine &current  = scalarLines[index];
          result.push_back(
              previous.text.empty() || current.text.empty() || previous.moreIndented || current.moreIndented ? '\n'
                                                                                                             : ' ');
        }
        result += scalarLines[index].text;
      }
      if (!scalarLines.empty())
        result.push_back('\n');
    }

    if (chompingMethod == '-') {
      while (!result.empty() && result.back() == '\n')
        result.pop_back();
    } else if (chompingMethod != '+') {
      while (!result.empty() && result.back() == '\n')
        result.pop_back();
      if (!scalarLines.empty())
        result.push_back('\n');
    }
    return YamlValue(result);
  }
};

std::vector<std::string> readLines(std::istream &input) {
  std::vector<std::string> lines;
  std::string              line;
  while (std::getline(input, line))
    lines.push_back(line);
  return lines;
}

} // namespace

YamlValue YamlParser::parseFile(const std::string &filename) const {
  std::ifstream input(filename);
  if (!input.is_open())
    throw FileException(filename);
  return parse(input);
}

YamlValue YamlParser::parse(std::istream &input) const {
  DocumentParser parser(readLines(input));
  return parser.parseDocument();
}

YamlValue YamlParser::parseText(const std::string &yamlText) const {
  std::istringstream input(yamlText);
  return parse(input);
}

} // namespace yamlparser
