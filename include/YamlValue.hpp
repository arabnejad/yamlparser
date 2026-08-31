#pragma once

#include "YamlException.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace yamlparser {

class YamlValue;

using YamlSequence = std::vector<YamlValue>;
using YamlMapping  = std::map<std::string, YamlValue>;

/** A scalar, sequence, mapping, or null value in a YAML document. */
class YamlValue {
public:
  enum class Type { Null, String, Double, Integer, Boolean, Sequence, Mapping };

  YamlValue();
  explicit YamlValue(const char *value);
  explicit YamlValue(const std::string &value);
  explicit YamlValue(double value);
  explicit YamlValue(int value);
  explicit YamlValue(bool value);
  explicit YamlValue(const YamlSequence &value);
  explicit YamlValue(const YamlMapping &value);

  YamlValue(const YamlValue &other);
  YamlValue(YamlValue &&other) noexcept;
  YamlValue &operator=(const YamlValue &other);
  YamlValue &operator=(YamlValue &&other) noexcept;
  ~YamlValue();

  Type type() const noexcept;

  bool isNull() const noexcept;
  bool isString() const noexcept;
  bool isDouble() const noexcept;
  bool isInteger() const noexcept;
  bool isBoolean() const noexcept;
  bool isSequence() const noexcept;
  bool isMapping() const noexcept;
  bool isScalar() const noexcept;

  const std::string  &asString() const;
  double              asDouble() const;
  int                 asInteger() const;
  bool                asBoolean() const;
  const YamlSequence &asSequence() const;
  const YamlMapping  &asMapping() const;

  const YamlValue &at(std::size_t index) const;
  const YamlValue &at(const std::string &key) const;

  void swap(YamlValue &other) noexcept;

private:
  struct Storage;
  std::unique_ptr<Storage> m_storage;
};

} // namespace yamlparser
