#pragma once
#include "YamlException.hpp"
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <memory>

/**
 * @brief Core YAML value representation system
 *
 * This file defines the type system for representing YAML values:
 * - YamlElement: A type-safe variant class for any YAML value
 * - YamlSeq: Vector-based sequence type
 * - YamlMap: String-keyed mapping type
 */

namespace yamlparser {

class YamlElement;
class YamlItem;
/**
 * @brief YAML sequence type (ordered list of values)
 * Implemented as a vector for:
 * - Ordered elements
 * - Random access
 * - Efficient insertion at end
 */
using YamlSeq = std::vector<YamlItem>;

/**
 * @brief YAML mapping type (string-keyed dictionary)
 * Implemented as a map for:
 * - Key uniqueness
 * - Ordered keys
 * - Efficient key lookup
 */
using YamlMap = std::map<std::string, YamlItem>;

// YamlElement: Holds any YAML value (scalar, sequence, or mapping)
class YamlElement {
public:
  /**
   * @brief Type tag for the contained value
   *
   * Used to:
   * - Determine which union member is active
   * - Enable type-safe access to values
   * - Support YAML type system mapping
   */
  enum class ElementType {
    NONE,   ///< No value (null in YAML)
    STRING, ///< UTF-8 string value
    DOUBLE, ///< double precision float
    INT,    ///< signed integer
    BOOL,   ///< Boolean true/false
    SEQ,    ///< Sequence (vector of values)
    MAP     ///< Mapping (string -> value)
  };
  ElementType type;

  /**
   * @brief Internal tagged union storage for YAML values
   *
   * This structure holds all possible YAML value types in a union-like structure.
   * Only one member is valid at a time, determined by the ElementType tag.
   */
  struct Data {
    /** @brief String value storage (valid when type == STRING) */
    std::unique_ptr<std::string> str;
    /** @brief Double value storage (valid when type == DOUBLE) */
    double d;
    /** @brief Integer value storage (valid when type == INT) */
    int i;
    /** @brief Boolean value storage (valid when type == BOOL) */
    bool b;
    /** @brief Sequence value storage (valid when type == SEQ) */
    std::unique_ptr<YamlSeq> seq;
    /** @brief Mapping value storage (valid when type == MAP) */
    std::unique_ptr<YamlMap> map;
    Data() : str(nullptr), d(0), i(0), b(false), seq(nullptr), map(nullptr) {}
  } data;

  /**
   * @name Value Constructors
   * Constructors for creating YamlElements of different types
   * @{
   */
  /** @brief Create a null/none value */
  YamlElement();
  /** @brief Create a string value */
  explicit YamlElement(const std::string &s);
  /** @brief Create a double value */
  explicit YamlElement(double d);
  /** @brief Create an integer value */
  explicit YamlElement(int i);
  /** @brief Create a boolean value */
  explicit YamlElement(bool b);
  /** @brief Create a sequence value */
  explicit YamlElement(const YamlSeq &seq);
  /** @brief Create a mapping value */
  explicit YamlElement(const YamlMap &map);
  /** @} */

  /**
   * @name Rule of Five Implementation
   * Special member functions for proper resource management
   * @{
   */
  /** @brief Copy constructor - deep copies all resources */
  YamlElement(const YamlElement &other);
  /** @brief Move constructor - transfers ownership */
  YamlElement(YamlElement &&other) noexcept;
  /** @brief Copy assignment - uses copy-and-swap idiom */
  YamlElement &operator=(const YamlElement &other);
  /** @brief Move assignment - transfers ownership */
  YamlElement &operator=(YamlElement &&other) noexcept;
  /** @brief Destructor - cleans up resources */
  ~YamlElement() = default;
  /** @} */

  void swap(YamlElement &other);

  /**
   * @name Value Access Methods
   * Type-safe accessors for contained values
   * @{
   */
  const std::string &asString() const;

  const double &asDouble() const;

  const int &asInt() const;

  const bool &asBool() const;

  const YamlSeq &asSeq() const;

  const YamlMap &asMap() const;
  /** @} */

  /**
   * @name Type Check Methods
   * Safe type checking before value access
   * @{
   */
  bool isString() const;

  bool isDouble() const;

  bool isInt() const;

  bool isBool() const;

  bool isSeq() const;

  bool isMap() const;

  bool isScalar() const;
  /** @} */

  /**
   * @name Static Utility Methods
   * Safe access methods for collections
   * @{
   */
  static const YamlItem &at(const YamlSeq &seq, size_t index);

  static const YamlItem &at(const YamlMap &map, const std::string &key);
  /** @} */
};

/**
 * @brief Wrapper class for recursive YAML structures
 *
 * This class wraps YamlElement to enable recursive data structures like
 * sequences of sequences or mappings containing sequences. It breaks
 * circular dependencies in the type system.
 */
class YamlItem {
public:
  /** @brief The wrapped YAML element */
  YamlElement value;

  /** @brief Default constructor creates an empty item */
  YamlItem() = default;

  /** @brief Create an item wrapping the given element
   * @param element The YAML element to wrap
   */
  YamlItem(const YamlElement &element) : value(element) {}
};

} // namespace yamlparser
