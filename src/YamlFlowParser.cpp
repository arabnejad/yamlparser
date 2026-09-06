#include "YamlFlowParser.hpp"

#include "YamlException.hpp"
#include "YamlScalarParser.hpp"

#include <cctype>
#include <set>
#include <utility>
#include <vector>

namespace yamlparser {
namespace internal {
namespace {

// Describes what one token means. For example, `[` becomes LeftBracket,
// while `localhost` becomes Scalar.
enum class FlowTokenKind {
  LeftBracket,
  RightBracket,
  LeftBrace,
  RightBrace,
  Comma,
  Colon,
  QuestionMark,
  Anchor,
  Alias,
  Tag,
  Scalar,
  End
};

// Stores a token together with its original source position for clear errors.
struct FlowToken {
  FlowTokenKind  kind;
  std::string    text;
  SourcePosition sourcePosition;
};

// Remembers an opening `[` or `{` until its matching delimiter is found.
struct OpeningDelimiter {
  char           openingSymbol;
  SourcePosition sourcePosition;
};

// Removes whitespace around token text without changing whitespace inside it.
// For example, `  hello world  ` becomes `hello world`.
std::string trimWhitespace(const std::string &text) {
  const std::size_t first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    return "";
  const std::size_t last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1U);
}

// Converts one flow expression into position-aware tokens and rejects
// mismatched delimiters before parsing begins.
class FlowCollectionTokenizer {
public:
  // Starts tokenizing at the expression's real position in the YAML document.
  FlowCollectionTokenizer(const std::string &expression, SourcePosition sourcePosition)
      : m_expression(expression), m_currentPosition(sourcePosition) {}

  // Reads the whole expression and returns its tokens in source order.
  // For example, `[one, two]` becomes `[`, `one`, `,`, `two`, `]`, and End.
  std::vector<FlowToken> tokenize() {
    while (!atEnd()) {
      skipWhitespace();
      if (atEnd())
        break;

      const char character = currentCharacter();
      switch (character) {
      case '[':
        addOpeningToken(FlowTokenKind::LeftBracket, '[');
        break;
      case ']':
        addClosingToken(FlowTokenKind::RightBracket, ']', '[');
        break;
      case '{':
        addOpeningToken(FlowTokenKind::LeftBrace, '{');
        break;
      case '}':
        addClosingToken(FlowTokenKind::RightBrace, '}', '{');
        break;
      case ',':
        addSingleCharacterToken(FlowTokenKind::Comma);
        break;
      case ':':
        addSingleCharacterToken(FlowTokenKind::Colon);
        break;
      case '?':
        addSingleCharacterToken(FlowTokenKind::QuestionMark);
        break;
      case '&':
        addPrefixedToken(FlowTokenKind::Anchor, "Anchor name cannot be empty");
        break;
      case '*':
        addPrefixedToken(FlowTokenKind::Alias, "Alias name cannot be empty");
        break;
      case '!':
        addPrefixedToken(FlowTokenKind::Tag, "Tag cannot be empty");
        break;
      case '\'':
      case '"':
        addQuotedScalarToken();
        break;
      default:
        addPlainScalarToken();
        break;
      }
    }

    if (!m_openingDelimiters.empty()) {
      const OpeningDelimiter &opening = m_openingDelimiters.back();
      throw SyntaxException(std::string("Unmatched '") + opening.openingSymbol + "'", opening.sourcePosition.lineNumber,
                            opening.sourcePosition.columnNumber);
    }

    m_tokens.push_back(FlowToken{FlowTokenKind::End, "", m_currentPosition});
    return m_tokens;
  }

private:
  const std::string            &m_expression;
  std::size_t                   m_characterIndex = 0U;
  SourcePosition                m_currentPosition;
  std::vector<FlowToken>        m_tokens;
  std::vector<OpeningDelimiter> m_openingDelimiters;

  // Returns true when every character in the expression has been read.
  bool atEnd() const noexcept {
    return m_characterIndex >= m_expression.size();
  }

  // Returns the character currently being examined without moving forward.
  char currentCharacter() const {
    return m_expression[m_characterIndex];
  }

  // Moves forward by one character and keeps the source position accurate.
  // For example, moving past a newline starts the next line at column 1.
  void moveToNextCharacter() {
    if (currentCharacter() == '\n') {
      ++m_currentPosition.lineNumber;
      m_currentPosition.columnNumber = 1U;
    } else {
      ++m_currentPosition.columnNumber;
    }
    ++m_characterIndex;
  }

  // Moves past spaces and other whitespace between flow tokens.
  void skipWhitespace() {
    while (!atEnd() && std::isspace(static_cast<unsigned char>(currentCharacter())) != 0)
      moveToNextCharacter();
  }

  // Adds punctuation such as `,` or `:` as one token, then moves past it.
  void addSingleCharacterToken(FlowTokenKind kind) {
    const SourcePosition sourcePosition = m_currentPosition;
    const std::string    text(1U, currentCharacter());
    moveToNextCharacter();
    m_tokens.push_back(FlowToken{kind, text, sourcePosition});
  }

  // Adds `[` or `{` and remembers it so a matching closing symbol is required.
  void addOpeningToken(FlowTokenKind kind, char symbol) {
    m_openingDelimiters.push_back(OpeningDelimiter{symbol, m_currentPosition});
    addSingleCharacterToken(kind);
  }

  // Adds `]` or `}` after checking that it closes the latest open collection.
  // For example, `[value}` fails because `}` cannot close `[`.
  void addClosingToken(FlowTokenKind kind, char symbol, char expectedOpening) {
    if (m_openingDelimiters.empty() || m_openingDelimiters.back().openingSymbol != expectedOpening)
      throw SyntaxException(std::string("Unexpected '") + symbol + "'", m_currentPosition.lineNumber,
                            m_currentPosition.columnNumber);
    m_openingDelimiters.pop_back();
    addSingleCharacterToken(kind);
  }

  // Decides whether a colon separates a mapping key from its value.
  // The colon in `host: localhost` separates values, while the colon in
  // `https://example.com` remains part of the scalar text.
  bool isMappingSeparatorAt(std::size_t colonIndex) const {
    if (colonIndex + 1U >= m_expression.size())
      return true;
    const char next = m_expression[colonIndex + 1U];

    // A colon followed by a value boundary separates a mapping key from its
    // value. Other colons remain part of plain scalars such as https://host.
    const bool nextStartsNonPlainValue =
        next == '[' || next == '{' || next == '\'' || next == '"' || next == '&' || next == '*' || next == '!';
    return std::isspace(static_cast<unsigned char>(next)) != 0 || nextStartsNonPlainValue || next == ',' ||
           next == ']' || next == '}';
  }

  // Returns true when an anchor, alias, or tag name must end.
  // For example, the comma ends the alias name in `[*defaults, local]`.
  bool isPrefixedNameBoundary(char character) const {
    return std::isspace(static_cast<unsigned char>(character)) != 0 || character == '[' || character == ']' ||
           character == '{' || character == '}' || character == ',' || character == ':' || character == '?';
  }

  // Reads a name after `&`, `*`, or `!` and stores it as one token.
  // For example, `&defaults` stores `defaults` as an Anchor token.
  void addPrefixedToken(FlowTokenKind kind, const std::string &emptyNameMessage) {
    const SourcePosition tokenPosition = m_currentPosition;
    moveToNextCharacter();

    const std::size_t nameStartIndex = m_characterIndex;
    while (!atEnd() && !isPrefixedNameBoundary(currentCharacter()))
      moveToNextCharacter();
    if (m_characterIndex == nameStartIndex)
      throw SyntaxException(emptyNameMessage, tokenPosition.lineNumber, tokenPosition.columnNumber);

    m_tokens.push_back(
        FlowToken{kind, m_expression.substr(nameStartIndex, m_characterIndex - nameStartIndex), tokenPosition});
  }

  // Reads one complete quoted scalar while respecting escapes and doubled
  // single quotes. For example, `"a,b"` remains one scalar rather than being
  // split at the comma.
  void addQuotedScalarToken() {
    const std::size_t    tokenStartIndex = m_characterIndex;
    const SourcePosition tokenPosition   = m_currentPosition;
    const char           quote           = currentCharacter();
    moveToNextCharacter();

    while (!atEnd()) {
      const char character = currentCharacter();
      if (character == '\n' || character == '\r')
        throw SyntaxException("Quoted strings must close on the same line", tokenPosition.lineNumber,
                              tokenPosition.columnNumber);

      if (quote == '"' && character == '\\') {
        moveToNextCharacter();
        if (atEnd())
          throw SyntaxException("Incomplete escape sequence", m_currentPosition.lineNumber,
                                m_currentPosition.columnNumber);
        moveToNextCharacter();
        continue;
      }

      if (character == quote) {
        moveToNextCharacter();
        if (quote == '\'' && !atEnd() && currentCharacter() == '\'') {
          moveToNextCharacter();
          continue;
        }
        m_tokens.push_back(FlowToken{FlowTokenKind::Scalar,
                                     m_expression.substr(tokenStartIndex, m_characterIndex - tokenStartIndex),
                                     tokenPosition});
        return;
      }
      moveToNextCharacter();
    }

    throw SyntaxException("Quoted strings must close on the same line", tokenPosition.lineNumber,
                          tokenPosition.columnNumber);
  }

  // Reads unquoted text up to the next flow separator and adds a Scalar token.
  // For example, `localhost,` adds `localhost` and leaves the comma for the
  // next tokenizer step.
  void addPlainScalarToken() {
    const std::size_t    tokenStartIndex = m_characterIndex;
    const SourcePosition tokenPosition   = m_currentPosition;

    while (!atEnd()) {
      const char character = currentCharacter();
      if (character == '[' || character == ']' || character == '{' || character == '}' || character == ',')
        break;
      if (character == ':' && isMappingSeparatorAt(m_characterIndex))
        break;
      moveToNextCharacter();
    }

    const std::string text = trimWhitespace(m_expression.substr(tokenStartIndex, m_characterIndex - tokenStartIndex));
    if (text.empty())
      throw SyntaxException("Expected a scalar value", tokenPosition.lineNumber, tokenPosition.columnNumber);
    m_tokens.push_back(FlowToken{FlowTokenKind::Scalar, text, tokenPosition});
  }
};

// Parses nested flow values using this small recursive-descent grammar.
// For example, parsing `{values: [1, 2]}` calls the mapping parser, which calls
// the same value parser again when it reaches the nested sequence.
//   value    := scalar | sequence | mapping | anchor value | alias
//   sequence := '[' (value (',' value)*)? ']'
//   mapping  := '{' (scalar ':' value? (',' scalar ':' value?)*)? '}'
//   merge    := alias | '[' alias (',' alias)* ']'
class FlowValueParser {
public:
  // Creates a parser for an already-tokenized expression and shares the
  // current document's anchor store so `*name` can resolve `&name`.
  FlowValueParser(std::vector<FlowToken> tokens, DocumentAnchorStore &anchorStore)
      : m_tokens(std::move(tokens)), m_anchorStore(anchorStore) {}

  // Parses the root value and rejects anything left after it.
  // For example, `[1, 2] extra` fails because `extra` is trailing content.
  YamlValue parse() {
    YamlValue parsedValue = parseValue();
    if (currentToken().kind != FlowTokenKind::End)
      throwSyntaxError("Unexpected content after flow collection", currentToken());
    return parsedValue;
  }

  // Parses a standalone merge value and rejects trailing tokens.
  // For example, both `*defaults` and `[*primary, *fallback]` are accepted.
  std::vector<AliasReference> parseMergeAliases() {
    std::vector<AliasReference> aliases = parseMergeAliasValue();
    if (currentToken().kind != FlowTokenKind::End)
      throwSyntaxError("Unexpected content after merge value", currentToken());
    return aliases;
  }

private:
  std::vector<FlowToken> m_tokens;
  std::size_t            m_currentTokenIndex = 0U;
  DocumentAnchorStore   &m_anchorStore;

  // Returns the next token without consuming it.
  const FlowToken &currentToken() const {
    return m_tokens[m_currentTokenIndex];
  }

  // Consumes one required token or reports the supplied syntax error.
  // For example, after a mapping key this requires `:` before its value.
  FlowToken consumeExpected(FlowTokenKind expectedKind, const std::string &errorMessage) {
    if (currentToken().kind != expectedKind)
      throwSyntaxError(errorMessage, currentToken());
    return m_tokens[m_currentTokenIndex++];
  }

  // Consumes a token only when it has the requested kind.
  // For example, after `[` it consumes `]` immediately for an empty sequence;
  // otherwise it leaves the next value untouched and returns false.
  bool consumeIfPresent(FlowTokenKind kind) {
    if (currentToken().kind != kind)
      return false;
    ++m_currentTokenIndex;
    return true;
  }

  // Stops parsing with a syntax error at the token's original line and column.
  [[noreturn]] void throwSyntaxError(const std::string &message, const FlowToken &token) const {
    throw SyntaxException(message, token.sourcePosition.lineNumber, token.sourcePosition.columnNumber);
  }

  // Parses any value allowed inside a flow collection. Nested sequences and
  // mappings come back through this function, which provides the recursion.
  YamlValue parseValue() {
    switch (currentToken().kind) {
    case FlowTokenKind::LeftBracket:
      return YamlValue(parseSequence());
    case FlowTokenKind::LeftBrace:
      return YamlValue(parseMapping());
    case FlowTokenKind::Scalar: {
      const FlowToken token = consumeExpected(FlowTokenKind::Scalar, "Expected a scalar");
      if (!token.text.empty() && (token.text.front() == '\'' || token.text.front() == '"'))
        return YamlValue(parseQuotedScalar(token.text, token.sourcePosition));
      return parsePlainScalarValue(token.text);
    }
    case FlowTokenKind::Anchor:
      return parseAnchoredValue();
    case FlowTokenKind::Alias:
      return resolveAlias();
    case FlowTokenKind::Tag:
      throwSyntaxError("Tags are not supported", currentToken());
    case FlowTokenKind::QuestionMark:
      throwSyntaxError("Complex mapping keys are not supported", currentToken());
    default:
      throwSyntaxError("Expected a value", currentToken());
    }
  }

  // Parses the value after an anchor and saves a copy under its name.
  // For example, `&defaults {port: 80}` stores the parsed mapping as defaults.
  YamlValue parseAnchoredValue() {
    const FlowToken anchor = consumeExpected(FlowTokenKind::Anchor, "Expected an anchor");
    YamlValue       value  = parseValue();
    m_anchorStore.defineAnchor(anchor.text, value);
    return value;
  }

  // Looks up an alias in the current document's anchors.
  // For example, `*defaults` returns the value previously stored by
  // `&defaults`.
  YamlValue resolveAlias() {
    const FlowToken alias = consumeExpected(FlowTokenKind::Alias, "Expected an alias");
    return m_anchorStore.resolveAlias(alias.text);
  }

  // Parses values between `[` and `]`, requiring commas between each item.
  // For example, `[1, {name: first}]` returns a two-item sequence.
  YamlSequence parseSequence() {
    consumeExpected(FlowTokenKind::LeftBracket, "Expected '['");
    YamlSequence sequenceItems;
    if (consumeIfPresent(FlowTokenKind::RightBracket))
      return sequenceItems;

    while (true) {
      if (currentToken().kind == FlowTokenKind::Comma || currentToken().kind == FlowTokenKind::RightBracket)
        throwSyntaxError("Flow sequence contains an empty item", currentToken());
      sequenceItems.push_back(parseValue());

      if (consumeIfPresent(FlowTokenKind::RightBracket))
        return sequenceItems;
      consumeExpected(FlowTokenKind::Comma, "Expected ',' or ']' after flow sequence item");
      if (currentToken().kind == FlowTokenKind::RightBracket)
        throwSyntaxError("Flow sequence contains an empty item", currentToken());
    }
  }

  // Parses a plain or quoted scalar key. Collection keys are rejected because
  // the current YamlMapping type stores string keys only.
  std::string parseMappingKey() {
    if (currentToken().kind == FlowTokenKind::QuestionMark)
      throwSyntaxError("Complex mapping keys are not supported", currentToken());
    const FlowToken key = consumeExpected(FlowTokenKind::Scalar, "Flow mapping key must be a scalar");
    if (!key.text.empty() && (key.text.front() == '\'' || key.text.front() == '"'))
      return parseQuotedScalar(key.text, key.sourcePosition);
    return key.text;
  }

  // Reads the alias at the current token and moves to the following token.
  // Keeping the source position allows later validation errors to identify the
  // exact alias that failed.
  AliasReference consumeMergeAlias() {
    if (currentToken().kind != FlowTokenKind::Alias)
      throwSyntaxError("Merge list items must be aliases", currentToken());

    const FlowToken alias = consumeExpected(FlowTokenKind::Alias, "Expected a merge alias");
    return AliasReference{alias.text, alias.sourcePosition};
  }

  // Parses one alias or a bracketed list of aliases as a merge value.
  // The returned order is preserved so the anchor store can apply YAML's
  // earlier-alias precedence rule.
  std::vector<AliasReference> parseMergeAliasValue() {
    std::vector<AliasReference> aliases;
    if (currentToken().kind == FlowTokenKind::Alias) {
      aliases.push_back(consumeMergeAlias());
      return aliases;
    }

    if (!consumeIfPresent(FlowTokenKind::LeftBracket))
      throwSyntaxError("Merge value must be an alias or a list of aliases", currentToken());
    if (currentToken().kind == FlowTokenKind::RightBracket)
      throwSyntaxError("Merge alias list cannot be empty", currentToken());

    while (true) {
      aliases.push_back(consumeMergeAlias());
      if (consumeIfPresent(FlowTokenKind::RightBracket))
        return aliases;
      consumeExpected(FlowTokenKind::Comma, "Expected ',' or ']' after merge alias");
      if (currentToken().kind == FlowTokenKind::RightBracket)
        throwSyntaxError("Merge alias list contains an empty item", currentToken());
    }
  }

  // Adds one ordinary key/value pair and rejects duplicate explicit keys.
  void addExplicitMappingEntry(const std::string &key, YamlValue value, YamlMapping &mapping,
                               std::set<std::string> &explicitKeys, const FlowToken &keyToken) {
    if (!explicitKeys.insert(key).second)
      throwSyntaxError("Duplicate mapping key: '" + key + "'", keyToken);
    mapping[key] = std::move(value);
  }

  // Parses entries between `{` and `}`, requiring `:` after keys and commas
  // between entries. For example, `{host: localhost, port: 8080}` returns a
  // mapping with two values.
  YamlMapping parseMapping() {
    consumeExpected(FlowTokenKind::LeftBrace, "Expected '{'");
    YamlMapping           mapping;
    std::set<std::string> explicitKeys;
    if (consumeIfPresent(FlowTokenKind::RightBrace))
      return mapping;

    while (true) {
      if (currentToken().kind == FlowTokenKind::Comma || currentToken().kind == FlowTokenKind::RightBrace)
        throwSyntaxError("Flow mapping contains an empty entry", currentToken());

      const FlowToken   keyToken   = currentToken();
      const bool        isMergeKey = keyToken.kind == FlowTokenKind::Scalar && keyToken.text == "<<";
      const std::string key        = parseMappingKey();
      consumeExpected(FlowTokenKind::Colon, "Expected ':' after flow mapping key");

      if (isMergeKey) {
        const std::vector<AliasReference> aliases = parseMergeAliasValue();
        m_anchorStore.mergeMappingsFromAliases(aliases, mapping);
      } else {
        // As in block mappings, an omitted value is YAML null. A comma still
        // separates this entry from the next one; a closing brace ends it.
        YamlValue value;
        if (currentToken().kind != FlowTokenKind::Comma && currentToken().kind != FlowTokenKind::RightBrace)
          value = parseValue();
        addExplicitMappingEntry(key, std::move(value), mapping, explicitKeys, keyToken);
      }

      if (consumeIfPresent(FlowTokenKind::RightBrace))
        return mapping;
      consumeExpected(FlowTokenKind::Comma, "Expected ',' or '}' after flow mapping entry");
      if (currentToken().kind == FlowTokenKind::RightBrace)
        throwSyntaxError("Flow mapping contains an empty entry", currentToken());
    }
  }
};

} // namespace

YamlValue parseFlowCollectionValue(const std::string &expression, SourcePosition sourcePosition,
                                   DocumentAnchorStore &anchorStore) {
  FlowCollectionTokenizer tokenizer(expression, sourcePosition);
  return FlowValueParser(tokenizer.tokenize(), anchorStore).parse();
}

std::vector<AliasReference> parseFlowMergeAliases(const std::string &expression, SourcePosition sourcePosition,
                                                  DocumentAnchorStore &anchorStore) {
  FlowCollectionTokenizer tokenizer(expression, sourcePosition);
  return FlowValueParser(tokenizer.tokenize(), anchorStore).parseMergeAliases();
}

} // namespace internal
} // namespace yamlparser
