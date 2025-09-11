
#include "YamlElement.hpp"
#include "YamlException.hpp"
#include <stdexcept>

// YamlElement implementation - A type-safe variant class for YAML values
// Uses a tagged union pattern with std::unique_ptr for dynamic memory management
// Supports: string, double, int, bool, sequence (vector), map (string->value), and none/null
// Memory safety is guaranteed through RAII and the use of smart pointers
// Type safety is enforced through exception throwing on invalid access

namespace yamlparser {

/**
 * @brief Default constructor creates a null/none YAML value
 */
YamlElement::YamlElement() : type(ElementType::NONE), data() {}

/**
 * @brief Constructs a string YAML element
 * @param s String value to store
 */
YamlElement::YamlElement(const std::string &s) : type(ElementType::STRING), data() {
  data.str = s;
}

/**
 * @brief Constructs a floating-point YAML element
 * @param d Double value to store
 */
YamlElement::YamlElement(double d) : type(ElementType::DOUBLE), data() {
  data.d = d;
}

/**
 * @brief Constructs an integer YAML element
 * @param i Integer value to store
 */
YamlElement::YamlElement(int i) : type(ElementType::INT), data() {
  data.i = i;
}

/**
 * @brief Constructs a boolean YAML element
 * @param b Boolean value to store
 */
YamlElement::YamlElement(bool b) : type(ElementType::BOOL), data() {
  data.b = b;
}

/**
 * @brief Constructs a sequence YAML element
 * @param seq Vector of YAML elements to store
 * @details Creates a deep copy of the input sequence
 */
YamlElement::YamlElement(const YamlSeq &seq) : type(ElementType::SEQ), data() {
  data.seq = std::make_unique<YamlSeq>(seq);
}

/**
 * @brief Constructs a mapping YAML element
 * @param map Map of string to YAML elements to store
 * @details Creates a deep copy of the input mapping
 */
YamlElement::YamlElement(const YamlMap &map) : type(ElementType::MAP), data() {
  data.map = std::make_unique<YamlMap>(map);
}

/**
 * @brief Copy constructor
 * @param other The YamlElement to copy
 * @details Performs a deep copy of the contained value based on its type:
 *          - For strings, sequences, and maps: creates new copies
 *          - For primitive types: copies the value directly
 */
YamlElement::YamlElement(const YamlElement &other) : type(other.type), data() {
  switch (type) {
  case ElementType::STRING:
    data.str = other.data.str;
    break;
  case ElementType::DOUBLE:
    data.d = other.data.d;
    break;
  case ElementType::INT:
    data.i = other.data.i;
    break;
  case ElementType::BOOL:
    data.b = other.data.b;
    break;
  case ElementType::SEQ:
    if (other.data.seq)
      data.seq = std::make_unique<YamlSeq>(*other.data.seq);
    break;
  case ElementType::MAP:
    if (other.data.map)
      data.map = std::make_unique<YamlMap>(*other.data.map);
    break;
  case ElementType::NONE:
  default:
    // Nothing to do; data is zero-initialized
    break;
  }
}

/**
 * @brief Move constructor
 * @param other The YamlElement to move from
 * @details Efficiently transfers ownership of resources from other to this
 *          Leaves other in a valid but unspecified state (NONE type)
 */
YamlElement::YamlElement(YamlElement &&other) noexcept : type(other.type), data(std::move(other.data)) {
  other.type = ElementType::NONE;
}

/**
 * @brief Copy assignment operator
 * @param other The YamlElement to copy from
 * @return Reference to this object
 * @details Uses copy-and-swap idiom for exception safety
 */
YamlElement &YamlElement::operator=(const YamlElement &other) {
  if (this != &other) {
    YamlElement temp(other); // Copy-construct
    swap(temp);              // Swap with temporary
  }
  return *this;
}

/**
 * @brief Move assignment operator
 * @param other The YamlElement to move from
 * @return Reference to this object
 * @details Efficiently transfers ownership from other to this
 *          Uses swap for simplicity and exception safety
 */
YamlElement &YamlElement::operator=(YamlElement &&other) noexcept {
  if (this != &other) {
    swap(other); // Steal other's resources
  }
  return *this;
}

/** @brief Check if value is a string */
bool YamlElement::isString() const {
  return type == ElementType::STRING;
}

/** @brief Check if value is a double */
bool YamlElement::isDouble() const {
  return type == ElementType::DOUBLE;
}

/** @brief Check if value is an integer */
bool YamlElement::isInt() const {
  return type == ElementType::INT;
}

/** @brief Check if value is a boolean */
bool YamlElement::isBool() const {
  return type == ElementType::BOOL;
}

/** @brief Check if value is a mapping (dictionary) */
bool YamlElement::isMap() const {
  return type == ElementType::MAP;
}

/** @brief Check if value is a sequence */
bool YamlElement::isSeq() const {
  return type == ElementType::SEQ;
}

/** @brief Check if value is a scalar (string, double, int, or bool)
 *
 * Scalar values are atomic/primitive types in YAML, as opposed to
 * collections like sequences or mappings.
 */
bool YamlElement::isScalar() const {
  return type == ElementType::STRING || type == ElementType::DOUBLE || type == ElementType::INT ||
         type == ElementType::BOOL;
}

/**
 * @brief Swaps the contents of this element with another
 * @param other The YamlElement to swap with
 * @details Provides strong exception guarantee as only std::swap operations are used
 *          Used by copy-and-swap idiom in assignment operators
 */
void YamlElement::swap(YamlElement &other) noexcept {
  std::swap(type, other.type);
  std::swap(data, other.data);
}

/**
 * @brief Accesses the string value
 * @return Const reference to the stored string
 * @throws TypeException if element is not a string
 */
const std::string &YamlElement::asString() const {
  if (type != ElementType::STRING) {
    throw TypeException("Expected string, but element is not a string");
  }
  return data.str;
}

/**
 * @brief Accesses the double value
 * @return value of the stored double
 * @throws TypeException if element is not a double
 */
double YamlElement::asDouble() const {
  if (type != ElementType::DOUBLE) {
    throw TypeException("Expected double, but element is not a double");
  }
  return data.d;
}
/**
 * @brief Accesses the integer value
 * @return value of the stored integer
 * @throws TypeException if element is not an integer
 */
int YamlElement::asInt() const {
  if (type != ElementType::INT) {
    throw TypeException("Expected integer, but element is not an integer");
  }
  return data.i;
}

/**
 * @brief Accesses the boolean value
 * @return value of the stored boolean
 * @throws TypeException if element is not a boolean
 */
bool YamlElement::asBool() const {
  if (type != ElementType::BOOL) {
    throw TypeException("Expected boolean, but element is not a boolean");
  }
  return data.b;
}
/**
 * @brief Accesses the sequence value
 * @return Const reference to the stored sequence
 * @throws TypeException if element is not a sequence
 */
const YamlSeq &YamlElement::asSeq() const {
  if (type != ElementType::SEQ) {
    throw TypeException("Expected sequence, but element is not a sequence");
  }
  return *data.seq;
}

/**
 * @brief Accesses the mapping value
 * @return Const reference to the stored mapping
 * @throws TypeException if element is not a mapping
 */
const YamlMap &YamlElement::asMap() const {
  if (type != ElementType::MAP) {
    throw TypeException("Expected mapping, but element is not a mapping");
  }
  return *data.map;
}

/**
 * @brief Safe access to sequence element by index
 * @param seq The sequence to access
 * @param index The index to access
 * @return Reference to the element at index
 * @throws IndexException if index is out of bounds
 */
const YamlItem &YamlElement::at(const YamlSeq &seq, size_t index) {
  if (index >= seq.size()) {
    throw IndexException(index, seq.size());
  }
  return seq[index];
}

/**
 * @brief Safe access to map element by key
 * @param map The map to access
 * @param key The key to access
 * @return Reference to the element with key
 * @throws KeyException if key is not found
 */
const YamlItem &YamlElement::at(const YamlMap &map, const std::string &key) {
  auto it = map.find(key);
  if (it == map.end()) {
    throw KeyException(key);
  }
  return it->second;
}

} // namespace yamlparser
