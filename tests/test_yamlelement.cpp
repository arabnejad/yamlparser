
#include <gtest/gtest.h>
#include "YamlElement.hpp"
#include "YamlException.hpp"
#include <string>

using namespace yamlparser;

class YamlElementTest : public ::testing::Test {
protected:
  void SetUp() override {
    // No special setup needed for YamlElement tests
  }

  void TearDown() override {
    // No cleanup needed for YamlElement tests
  }
};

TEST_F(YamlElementTest, DefaultConstructorCreatesNoneType) {
  // Test default constructor creates NONE type element
  // Verifies that a default-constructed YamlElement has NONE type,
  // representing an uninitialized or null YAML value

  // Setup: No special setup needed

  // Action: Create default YamlElement
  YamlElement element;

  // Assertion: Element should have NONE type
  EXPECT_EQ(element.type, YamlElement::ElementType::NONE);
}

TEST_F(YamlElementTest, StringConstructorSetsValue) {
  // Test string constructor and access methods
  // Verifies that YamlElement can be constructed from a string, correctly identifies
  // as string type, and provides proper access

  // Setup: Prepare test string
  const std::string testValue = "hello";

  // Action: Create YamlElement from string
  YamlElement element(testValue);

  // Assertions: Type and value should be correct
  EXPECT_EQ(element.type, YamlElement::ElementType::STRING);
  EXPECT_EQ(element.asString(), testValue);
  EXPECT_TRUE(element.isString());
  EXPECT_TRUE(element.isScalar());
}

TEST_F(YamlElementTest, IntConstructorSetsValue) {
  // Test integer constructor and access methods
  // Verifies that YamlElement can be constructed from an integer, correctly identifies
  // as int type, and provides proper access

  // Setup: Prepare test integer
  const int testValue = 42;

  // Action: Create YamlElement from integer
  YamlElement element(testValue);

  // Assertions: Type and value should be correct
  EXPECT_EQ(element.type, YamlElement::ElementType::INT);
  EXPECT_EQ(element.asInt(), testValue);
  EXPECT_TRUE(element.isInt());
  EXPECT_TRUE(element.isScalar());
}

TEST_F(YamlElementTest, DoubleConstructorSetsValue) {
  // Test double constructor and access methods
  // Verifies that YamlElement can be constructed from a double, correctly identifies
  // as double type, and provides proper access

  // Setup: Prepare test double value
  const double testValue = 3.14;

  // Action: Create YamlElement from double
  YamlElement element(testValue);

  // Assertions: Type and value should be correct
  EXPECT_EQ(element.type, YamlElement::ElementType::DOUBLE);
  EXPECT_DOUBLE_EQ(element.asDouble(), testValue);
  EXPECT_TRUE(element.isDouble());
  EXPECT_TRUE(element.isScalar());
}

TEST_F(YamlElementTest, BoolConstructorSetsValue) {
  // Test boolean constructor and access methods
  // Verifies that YamlElement can be constructed from a boolean, correctly identifies
  // as bool type, and provides proper access

  // Setup: Prepare test boolean value
  const bool testValue = true;

  // Action: Create YamlElement from boolean
  YamlElement element(testValue);

  // Assertions: Type and value should be correct
  EXPECT_EQ(element.type, YamlElement::ElementType::BOOL);
  EXPECT_TRUE(element.asBool());
  EXPECT_TRUE(element.isBool());
  EXPECT_TRUE(element.isScalar());
}

TEST_F(YamlElementTest, SeqConstructorSetsSequence) {
  // Test sequence constructor and access methods
  // Verifies that YamlElement can be constructed from a YamlSeq, correctly identifies
  // as sequence type, and provides proper access

  // Setup: Create sequence with test data
  YamlSeq seq;
  seq.push_back(YamlItem(YamlElement(1)));
  seq.push_back(YamlItem(YamlElement(2)));

  // Action: Create YamlElement from sequence
  YamlElement element(seq);

  // Assertions: Type and content should be correct
  EXPECT_EQ(element.type, YamlElement::ElementType::SEQ);
  EXPECT_EQ(element.asSeq().size(), 2);
  EXPECT_TRUE(element.isSeq());
}

TEST_F(YamlElementTest, MapConstructorSetsMap) {
  // Test map constructor and access methods
  // Verifies that YamlElement can be constructed from a YamlMap, correctly identifies
  // as map type, and provides proper access

  // Setup: Create map with test data
  YamlMap map;
  map["a"] = YamlItem(YamlElement(1));
  map["b"] = YamlItem(YamlElement(2));

  // Action: Create YamlElement from map
  YamlElement element(map);

  // Assertions: Type and content should be correct
  EXPECT_EQ(element.type, YamlElement::ElementType::MAP);
  EXPECT_EQ(element.asMap().size(), 2);
  EXPECT_TRUE(element.isMap());
}

TEST_F(YamlElementTest, CopyConstructorCopiesValue) {
  // Test copy constructor functionality
  // Verifies that YamlElement copy constructor creates an independent copy
  // with the same type and value as the original

  // Setup: Create original element with string value
  const std::string testValue = "foo";
  YamlElement       original(testValue);

  // Action: Create copy using copy constructor
  YamlElement copy(original);

  // Assertions: Copy should have same type and value
  EXPECT_EQ(copy.type, YamlElement::ElementType::STRING);
  EXPECT_EQ(copy.asString(), testValue);
}

TEST_F(YamlElementTest, AssignmentOperatorCopiesString) {
  // Test assignment operator with string elements
  // Verifies that assignment operator correctly copies string value
  // from one YamlElement to another

  // Setup: Create two string elements with different values
  YamlElement element1(std::string("foo"));
  YamlElement element2(std::string("bar"));

  // Action: Assign element2 to element1
  element1 = element2;

  // Assertions: element1 should now have element2's value
  EXPECT_EQ(element1.type, YamlElement::ElementType::STRING);
  EXPECT_EQ(element1.asString(), "bar");
}

TEST_F(YamlElementTest, AssignmentOperatorCopiesInt) {
  // Test assignment operator with integer elements
  // Verifies that assignment operator correctly copies integer value
  // from one YamlElement to another

  // Setup: Create two integer elements with different values
  YamlElement element1(1);
  YamlElement element2(2);

  // Action: Assign element2 to element1
  element1 = element2;

  // Assertions: element1 should now have element2's value
  EXPECT_EQ(element1.type, YamlElement::ElementType::INT);
  EXPECT_EQ(element1.asInt(), 2);
}

TEST_F(YamlElementTest, AssignmentOperatorCopiesDouble) {
  // Test assignment operator with double elements
  // Verifies that assignment operator correctly copies double value
  // from one YamlElement to another

  // Setup: Create two double elements with different values
  YamlElement element1(1.1);
  YamlElement element2(2.2);

  // Action: Assign element2 to element1
  element1 = element2;

  // Assertions: element1 should now have element2's value
  EXPECT_EQ(element1.type, YamlElement::ElementType::DOUBLE);
  EXPECT_DOUBLE_EQ(element1.asDouble(), 2.2);
}

TEST_F(YamlElementTest, AssignmentOperatorCopiesBool) {
  // Test assignment operator with boolean elements
  // Verifies that assignment operator correctly copies boolean value
  // from one YamlElement to another

  // Setup: Create two boolean elements with different values
  YamlElement element1(true);
  YamlElement element2(false);

  // Action: Assign element2 to element1
  element1 = element2;

  // Assertions: element1 should now have element2's value
  EXPECT_EQ(element1.type, YamlElement::ElementType::BOOL);
  EXPECT_FALSE(element1.asBool());
}

TEST_F(YamlElementTest, AssignmentOperatorCopiesSeq) {
  // Test assignment operator with sequence elements
  // Verifies that assignment operator correctly copies sequence content
  // from one YamlElement to another

  // Setup: Create two sequences with different content
  YamlSeq seq1, seq2;
  seq1.push_back(YamlItem(YamlElement(1)));
  seq2.push_back(YamlItem(YamlElement(2)));
  YamlElement element1(seq1);
  YamlElement element2(seq2);

  // Action: Assign element2 to element1
  element1 = element2;

  // Assertions: element1 should now have element2's sequence content
  EXPECT_EQ(element1.type, YamlElement::ElementType::SEQ);
  EXPECT_EQ(element1.asSeq().size(), 1);
  EXPECT_EQ(element1.asSeq()[0].value.asInt(), 2);
}

TEST_F(YamlElementTest, AssignmentOperatorCopiesMap) {
  // Test assignment operator with map elements
  // Verifies that assignment operator correctly copies map content
  // from one YamlElement to another

  // Setup: Create two maps with different content
  YamlMap map1, map2;
  map1["a"] = YamlItem(YamlElement(1));
  map2["b"] = YamlItem(YamlElement(2));
  YamlElement element1(map1);
  YamlElement element2(map2);

  // Action: Assign element2 to element1
  element1 = element2;

  // Assertions: element1 should now have element2's map content
  EXPECT_EQ(element1.type, YamlElement::ElementType::MAP);
  EXPECT_EQ(element1.asMap().size(), 1);
  EXPECT_TRUE(element1.asMap().count("b"));
  EXPECT_EQ(element1.asMap().at("b").value.asInt(), 2);
}

TEST_F(YamlElementTest, SwapExchangesTypesAndValues) {
  // Test swap functionality between elements of different types
  // Verifies that swap correctly exchanges the types and values
  // of two YamlElement objects

  // Setup: Create elements of different types
  YamlElement stringElement(std::string("foo"));
  YamlElement intElement(42);

  // Action: Swap the elements
  stringElement.swap(intElement);

  // Assertions: Types and values should be exchanged
  EXPECT_EQ(stringElement.type, YamlElement::ElementType::INT);
  EXPECT_EQ(intElement.type, YamlElement::ElementType::STRING);
  EXPECT_EQ(stringElement.asInt(), 42);
  EXPECT_EQ(intElement.asString(), "foo");

  // Action: Swap back to verify bidirectional operation
  stringElement.swap(intElement);

  // Assertions: Should be back to original state
  EXPECT_EQ(stringElement.type, YamlElement::ElementType::STRING);
  EXPECT_EQ(intElement.type, YamlElement::ElementType::INT);
  EXPECT_EQ(stringElement.asString(), "foo");
  EXPECT_EQ(intElement.asInt(), 42);
}

TEST_F(YamlElementTest, NoneTypeCopyAndAssignment) {
  // Test copy and assignment operations with NONE type elements
  // Verifies that NONE type elements (default constructed) can be
  // properly copied and assigned without errors

  // Setup: Create default (NONE type) elements
  YamlElement noneElement;

  // Action: Copy construct from NONE element
  YamlElement copyElement(noneElement);

  // Assertion: Copy should also be NONE type
  EXPECT_EQ(copyElement.type, YamlElement::ElementType::NONE);

  // Action: Assign NONE element to another NONE element
  YamlElement otherElement;
  noneElement = otherElement;

  // Assertion: Assignment should preserve NONE type
  EXPECT_EQ(noneElement.type, YamlElement::ElementType::NONE);
}

TEST_F(YamlElementTest, AssignmentWithInvalidEnumType) {
  // Test assignment operator with invalid enum value (edge case)
  // This test covers the default branch in the assignment operator
  // by using an artificially invalid enum value

  // Setup: Create elements and set invalid enum type
  YamlElement element1;
  YamlElement element2;
  element1.type = static_cast<YamlElement::ElementType>(-1);
  element2.type = static_cast<YamlElement::ElementType>(-1);

  // Action: Assign element with invalid type
  element1 = element2;

  // Assertion: Should preserve the invalid type without crashing
  EXPECT_EQ(element1.type, static_cast<YamlElement::ElementType>(-1));
}

TEST_F(YamlElementTest, SelfAssignmentHandledCorrectly) {
  // Test self-assignment in assignment operator
  // Verifies that self-assignment is handled correctly without
  // causing issues or unnecessary work

  // Setup: Create element with test value
  YamlElement  element(123);
  YamlElement &elementRef = element;

  // Action: Perform self-assignment
  element = elementRef;

  // Assertions: Should preserve original value and type
  EXPECT_EQ(element.type, YamlElement::ElementType::INT);
  EXPECT_EQ(element.asInt(), 123);
}

TEST_F(YamlElementTest, CopyConstructorHandlesNoneType) {
  // Test copy constructor with NONE type element
  // Verifies that copy constructor correctly handles NONE type elements
  // (default constructed elements) without errors

  // Setup: Create default NONE type element
  YamlElement noneElement;

  // Action: Copy construct from NONE element (triggers default case)
  YamlElement copyElement(noneElement);

  // Assertions: Should not crash and preserve NONE type
  EXPECT_EQ(copyElement.type, YamlElement::ElementType::NONE);
  EXPECT_EQ(noneElement.type, YamlElement::ElementType::NONE);
}

TEST_F(YamlElementTest, AssignmentOperatorHandlesNoneType) {
  // Test assignment operator with NONE type elements
  // Verifies that assignment operator correctly handles NONE type elements
  // (default constructed elements) without errors

  // Setup: Create default NONE type elements
  YamlElement element1;
  YamlElement element2;

  // Action: Assign NONE element to NONE element (triggers default case)
  element1 = element2;

  // Assertions: Should not crash and preserve NONE type
  EXPECT_EQ(element1.type, YamlElement::ElementType::NONE);
  EXPECT_EQ(element2.type, YamlElement::ElementType::NONE);
}

TEST_F(YamlElementTest, SwapComprehensiveVerification) {
  // Test swap functionality with verification
  // Additional verification of swap method to ensure it works correctly
  // in all scenarios and maintains data integrity

  // Setup: Create elements of different types with specific values
  YamlElement stringElement(std::string("foo"));
  YamlElement intElement(42);

  // Action: Perform swap operation
  stringElement.swap(intElement);

  // Assertions: Verify complete exchange
  EXPECT_EQ(stringElement.type, YamlElement::ElementType::INT);
  EXPECT_EQ(intElement.type, YamlElement::ElementType::STRING);
  EXPECT_EQ(stringElement.asInt(), 42);
  EXPECT_EQ(intElement.asString(), "foo");

  // Action: Swap back to test bidirectional functionality
  stringElement.swap(intElement);

  // Assertions: Verify return to original state
  EXPECT_EQ(stringElement.type, YamlElement::ElementType::STRING);
  EXPECT_EQ(intElement.type, YamlElement::ElementType::INT);
  EXPECT_EQ(stringElement.asString(), "foo");
  EXPECT_EQ(intElement.asInt(), 42);
}

TEST_F(YamlElementTest, TypeExceptionHandling) {
  // Test type exception handling for incorrect type access
  // Verifies that TypeException is properly thrown when attempting
  // to access a YamlElement with an incorrect type conversion method

  // Setup: Create elements of different types
  YamlElement stringElement(std::string("hello"));
  YamlElement intElement(42);
  YamlElement doubleElement(3.14);
  YamlElement boolElement(true);

  // Test: String element accessed with wrong type methods should throw
  EXPECT_THROW(stringElement.asInt(), TypeException);
  EXPECT_THROW(stringElement.asDouble(), TypeException);
  EXPECT_THROW(stringElement.asBool(), TypeException);
  EXPECT_THROW(stringElement.asSeq(), TypeException);
  EXPECT_THROW(stringElement.asMap(), TypeException);

  // Test: Int element accessed with wrong type methods should throw
  EXPECT_THROW(intElement.asString(), TypeException);
  EXPECT_THROW(intElement.asDouble(), TypeException);
  EXPECT_THROW(intElement.asBool(), TypeException);
  EXPECT_THROW(intElement.asSeq(), TypeException);
  EXPECT_THROW(intElement.asMap(), TypeException);

  // Test: Correct type access should not throw
  EXPECT_NO_THROW(stringElement.asString());
  EXPECT_NO_THROW(intElement.asInt());
  EXPECT_NO_THROW(doubleElement.asDouble());
  EXPECT_NO_THROW(boolElement.asBool());
}

// Negative/edge case tests for YamlElement
TEST_F(YamlElementTest, EmptyStringCreatesStringType) {
  YamlElement element(std::string(""));
  EXPECT_EQ(element.type, YamlElement::ElementType::STRING);
  EXPECT_EQ(element.asString(), "");
}

TEST_F(YamlElementTest, ZeroCreatesIntType) {
  YamlElement element(0);
  EXPECT_EQ(element.type, YamlElement::ElementType::INT);
  EXPECT_EQ(element.asInt(), 0);
}

TEST_F(YamlElementTest, FalseCreatesBoolType) {
  YamlElement element(false);
  EXPECT_EQ(element.type, YamlElement::ElementType::BOOL);
  EXPECT_FALSE(element.asBool());
}

TEST_F(YamlElementTest, EmptySequenceCreatesSeqType) {
  YamlSeq     seq;
  YamlElement element(seq);
  EXPECT_EQ(element.type, YamlElement::ElementType::SEQ);
  EXPECT_TRUE(element.asSeq().empty());
}

TEST_F(YamlElementTest, EmptyMapCreatesMapType) {
  YamlMap     map;
  YamlElement element(map);
  EXPECT_EQ(element.type, YamlElement::ElementType::MAP);
  EXPECT_TRUE(element.asMap().empty());
}

TEST_F(YamlElementTest, TypeConversionThrowsOnWrongType) {
  YamlElement element(42);
  EXPECT_THROW(element.asString(), TypeException);
  EXPECT_THROW(element.asDouble(), TypeException);
  EXPECT_THROW(element.asBool(), TypeException);
  EXPECT_THROW(element.asSeq(), TypeException);
  EXPECT_THROW(element.asMap(), TypeException);
}

TEST_F(YamlElementTest, AssignmentAndCopyWithInvalidEnumType) {
  YamlElement element1;
  YamlElement element2;
  element1.type = static_cast<YamlElement::ElementType>(-1);
  element2.type = static_cast<YamlElement::ElementType>(-1);
  // Assignment should preserve invalid type
  element1 = element2;
  EXPECT_EQ(element1.type, static_cast<YamlElement::ElementType>(-1));
  // Copy constructor should preserve invalid type
  YamlElement copy(element1);
  EXPECT_EQ(copy.type, static_cast<YamlElement::ElementType>(-1));
}

TEST_F(YamlElementTest, DeeplyNestedSequencesAndMaps) {
  YamlSeq seq1, seq2;
  seq2.push_back(YamlItem(YamlElement(42)));
  seq1.push_back(YamlItem(YamlElement(seq2)));
  YamlMap map1, map2;
  map2["inner"] = YamlItem(YamlElement(seq1));
  map1["outer"] = YamlItem(YamlElement(map2));
  YamlElement element(map1);
  EXPECT_EQ(element.type, YamlElement::ElementType::MAP);
  auto &outer = element.asMap();
  ASSERT_TRUE(outer.find("outer") != outer.end());
  auto &innerMap = outer.at("outer").value.asMap();
  ASSERT_TRUE(innerMap.find("inner") != innerMap.end());
  auto &nestedSeq = innerMap.at("inner").value.asSeq();
  ASSERT_FALSE(nestedSeq.empty());
  auto &deepSeq = nestedSeq[0].value.asSeq();
  ASSERT_FALSE(deepSeq.empty());
  EXPECT_EQ(deepSeq[0].value.asInt(), 42);
}