#pragma once

#include "YamlValue.hpp"

#include <gtest/gtest.h>

namespace yamlparser {
namespace test {

// Compares complete YAML trees, including every nested value and its type.
// This is stronger than checking only a few fields after a print/parse cycle.
inline void expectSameYamlValue(const YamlValue &expected, const YamlValue &actual) {
  ASSERT_EQ(expected.type(), actual.type());

  switch (expected.type()) {
  case YamlValue::Type::Null:
    return;
  case YamlValue::Type::String:
    EXPECT_EQ(expected.asString(), actual.asString());
    return;
  case YamlValue::Type::Double:
    EXPECT_DOUBLE_EQ(expected.asDouble(), actual.asDouble());
    return;
  case YamlValue::Type::Integer:
    EXPECT_EQ(expected.asInteger(), actual.asInteger());
    return;
  case YamlValue::Type::Boolean:
    EXPECT_EQ(expected.asBoolean(), actual.asBoolean());
    return;
  case YamlValue::Type::Sequence:
    ASSERT_EQ(expected.asSequence().size(), actual.asSequence().size());
    for (std::size_t index = 0U; index < expected.asSequence().size(); ++index)
      expectSameYamlValue(expected.at(index), actual.at(index));
    return;
  case YamlValue::Type::Mapping:
    ASSERT_EQ(expected.asMapping().size(), actual.asMapping().size());
    for (const auto &entry : expected.asMapping())
      expectSameYamlValue(entry.second, actual.at(entry.first));
    return;
  }
}

} // namespace test
} // namespace yamlparser
