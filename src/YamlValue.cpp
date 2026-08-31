#include "YamlValue.hpp"

namespace yamlparser {

struct YamlValue::Storage {
  Type         type = Type::Null;
  std::string  stringValue;
  double       doubleValue  = 0.0;
  int          integerValue = 0;
  bool         booleanValue = false;
  YamlSequence sequenceValue;
  YamlMapping  mappingValue;
};

namespace {
const char *typeName(YamlValue::Type type) {
  switch (type) {
  case YamlValue::Type::Null:
    return "null";
  case YamlValue::Type::String:
    return "string";
  case YamlValue::Type::Double:
    return "double";
  case YamlValue::Type::Integer:
    return "integer";
  case YamlValue::Type::Boolean:
    return "boolean";
  case YamlValue::Type::Sequence:
    return "sequence";
  case YamlValue::Type::Mapping:
    return "mapping";
  }
  return "unknown";
}
} // namespace

YamlValue::YamlValue() : m_storage(new Storage()) {}

YamlValue::YamlValue(const char *value) : YamlValue(std::string(value == nullptr ? "" : value)) {}

YamlValue::YamlValue(const std::string &value) : m_storage(new Storage()) {
  m_storage->type        = Type::String;
  m_storage->stringValue = value;
}

YamlValue::YamlValue(double value) : m_storage(new Storage()) {
  m_storage->type        = Type::Double;
  m_storage->doubleValue = value;
}

YamlValue::YamlValue(int value) : m_storage(new Storage()) {
  m_storage->type         = Type::Integer;
  m_storage->integerValue = value;
}

YamlValue::YamlValue(bool value) : m_storage(new Storage()) {
  m_storage->type         = Type::Boolean;
  m_storage->booleanValue = value;
}

YamlValue::YamlValue(const YamlSequence &value) : m_storage(new Storage()) {
  m_storage->type          = Type::Sequence;
  m_storage->sequenceValue = value;
}

YamlValue::YamlValue(const YamlMapping &value) : m_storage(new Storage()) {
  m_storage->type         = Type::Mapping;
  m_storage->mappingValue = value;
}

YamlValue::YamlValue(const YamlValue &other)
    : m_storage(other.m_storage ? new Storage(*other.m_storage) : new Storage()) {}

YamlValue::YamlValue(YamlValue &&other) noexcept = default;

YamlValue &YamlValue::operator=(const YamlValue &other) {
  if (this != &other) {
    YamlValue copy(other);
    swap(copy);
  }
  return *this;
}

YamlValue &YamlValue::operator=(YamlValue &&other) noexcept = default;

YamlValue::~YamlValue() = default;

YamlValue::Type YamlValue::type() const noexcept {
  return m_storage ? m_storage->type : Type::Null;
}

bool YamlValue::isNull() const noexcept {
  return type() == Type::Null;
}
bool YamlValue::isString() const noexcept {
  return type() == Type::String;
}
bool YamlValue::isDouble() const noexcept {
  return type() == Type::Double;
}
bool YamlValue::isInteger() const noexcept {
  return type() == Type::Integer;
}
bool YamlValue::isBoolean() const noexcept {
  return type() == Type::Boolean;
}
bool YamlValue::isSequence() const noexcept {
  return type() == Type::Sequence;
}
bool YamlValue::isMapping() const noexcept {
  return type() == Type::Mapping;
}

bool YamlValue::isScalar() const noexcept {
  return isString() || isDouble() || isInteger() || isBoolean();
}

const std::string &YamlValue::asString() const {
  if (!isString())
    throw TypeException(std::string("Expected string, found ") + typeName(type()));
  return m_storage->stringValue;
}

double YamlValue::asDouble() const {
  if (!isDouble())
    throw TypeException(std::string("Expected double, found ") + typeName(type()));
  return m_storage->doubleValue;
}

int YamlValue::asInteger() const {
  if (!isInteger())
    throw TypeException(std::string("Expected integer, found ") + typeName(type()));
  return m_storage->integerValue;
}

bool YamlValue::asBoolean() const {
  if (!isBoolean())
    throw TypeException(std::string("Expected boolean, found ") + typeName(type()));
  return m_storage->booleanValue;
}

const YamlSequence &YamlValue::asSequence() const {
  if (!isSequence())
    throw TypeException(std::string("Expected sequence, found ") + typeName(type()));
  return m_storage->sequenceValue;
}

const YamlMapping &YamlValue::asMapping() const {
  if (!isMapping())
    throw TypeException(std::string("Expected mapping, found ") + typeName(type()));
  return m_storage->mappingValue;
}

const YamlValue &YamlValue::at(std::size_t index) const {
  const YamlSequence &sequence = asSequence();
  if (index >= sequence.size())
    throw IndexException(index, sequence.size());
  return sequence[index];
}

const YamlValue &YamlValue::at(const std::string &key) const {
  const YamlMapping &mapping = asMapping();
  const auto         found   = mapping.find(key);
  if (found == mapping.end())
    throw KeyException(key);
  return found->second;
}

void YamlValue::swap(YamlValue &other) noexcept {
  m_storage.swap(other.m_storage);
}

} // namespace yamlparser
