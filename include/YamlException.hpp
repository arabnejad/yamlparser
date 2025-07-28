#pragma once
#include <exception>
#include <string>

namespace yamlparser {

/**
 * @brief Base exception class for all YAML parsing errors
 *
 * This is the base class for all exceptions thrown by the YAML parser.
 * It inherits from std::exception to provide compatibility with standard
 * C++ exception handling patterns.
 */
class YamlException : public std::exception {
public:
  /**
   * @brief Construct a new YAML exception
   * @param message Error description
   */
  explicit YamlException(const std::string &message) : m_message(message) {}

  /**
   * @brief Get the error message
   * @return const char* Error description
   */
  const char *what() const noexcept override {
    return m_message.c_str();
  }

private:
  std::string m_message;
};

/**
 * @brief Exception thrown when file I/O operations fail
 *
 * This exception is thrown when:
 * - File cannot be opened for reading
 * - File does not exist
 * - Insufficient permissions to read file
 */
class FileException : public YamlException {
public:
  explicit FileException(const std::string &filename) : YamlException("Cannot open or read file: " + filename) {}
};

/**
 * @brief Exception thrown when YAML syntax is invalid
 *
 * This exception is thrown when:
 * - Invalid YAML syntax is encountered
 * - Malformed structures
 * - Invalid indentation
 * - Unexpected characters or tokens
 */
class SyntaxException : public YamlException {
public:
  explicit SyntaxException(const std::string &message) : YamlException("YAML syntax error: " + message) {}

  SyntaxException(const std::string &message, size_t line)
      : YamlException("YAML syntax error at line " + std::to_string(line) + ": " + message) {}
};

/**
 * @brief Exception thrown when type operations are invalid
 *
 * This exception is thrown when:
 * - Calling asInt() on a string value
 * - Calling asString() on a sequence
 * - Type conversion failures
 * - Accessing wrong data type
 */
class TypeException : public YamlException {
public:
  explicit TypeException(const std::string &message) : YamlException("Type error: " + message) {}
};

/**
 * @brief Exception thrown when accessing non-existent keys
 *
 * This exception is thrown when:
 * - Accessing a key that doesn't exist in a map
 * - Using at() method with invalid key
 */
class KeyException : public YamlException {
public:
  explicit KeyException(const std::string &key) : YamlException("Key not found: '" + key + "'") {}
};

/**
 * @brief Exception thrown when accessing invalid sequence indices
 *
 * This exception is thrown when:
 * - Index is out of bounds for sequence
 * - Negative index (if applicable)
 */
class IndexException : public YamlException {
public:
  explicit IndexException(size_t index, size_t size)
      : YamlException("Index out of bounds: " + std::to_string(index) + " (sequence size: " + std::to_string(size) +
                      ")") {}
};

/**
 * @brief Exception thrown when value conversion fails
 *
 * This exception is thrown when:
 * - String cannot be converted to integer
 * - String cannot be converted to double
 * - Invalid boolean values
 */
class ConversionException : public YamlException {
public:
  explicit ConversionException(const std::string &value, const std::string &targetType)
      : YamlException("Cannot convert '" + value + "' to " + targetType) {}
};

/**
 * @brief Exception thrown when structure assumptions are violated
 *
 * This exception is thrown when:
 * - Expecting a map but getting a sequence
 * - Expecting a sequence but getting a map
 * - Root element type mismatch
 */
class StructureException : public YamlException {
public:
  explicit StructureException(const std::string &message) : YamlException("Structure error: " + message) {}
};

} // namespace yamlparser
