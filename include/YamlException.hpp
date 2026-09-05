#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

namespace yamlparser {

/** Base class for every exception reported by this library. */
class YamlException : public std::runtime_error {
public:
  explicit YamlException(const std::string &message) : std::runtime_error(message) {}
};

class FileException : public YamlException {
public:
  explicit FileException(const std::string &filename) : YamlException("Cannot open or read file: " + filename) {}
};

class SyntaxException : public YamlException {
public:
  explicit SyntaxException(const std::string &message) : YamlException("YAML syntax error: " + message) {}

  SyntaxException(const std::string &message, std::size_t lineNumber)
      : YamlException("YAML syntax error at line " + std::to_string(lineNumber) + ": " + message) {}

  SyntaxException(const std::string &message, std::size_t lineNumber, std::size_t columnNumber)
      : YamlException("YAML syntax error at line " + std::to_string(lineNumber) + ", column " +
                      std::to_string(columnNumber) + ": " + message) {}
};

class TypeException : public YamlException {
public:
  explicit TypeException(const std::string &message) : YamlException("Type error: " + message) {}
};

class KeyException : public YamlException {
public:
  explicit KeyException(const std::string &key) : YamlException("Key not found: '" + key + "'") {}
};

class IndexException : public YamlException {
public:
  IndexException(std::size_t index, std::size_t sequenceSize)
      : YamlException("Index out of bounds: " + std::to_string(index) +
                      " (sequence size: " + std::to_string(sequenceSize) + ")") {}
};

class ConversionException : public YamlException {
public:
  ConversionException(const std::string &value, const std::string &targetType)
      : YamlException("Cannot convert '" + value + "' to " + targetType) {}
};

} // namespace yamlparser
