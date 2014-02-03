#include "TestSuite.h"

void TestSuite::TestFunctions(void){
    UnitTest::SetPrefix("Swap");
    #ifdef JSON_LIBRARY
		  JSONNODE * test1 = json_new(JSON_NODE);
		  JSONNODE * test2 = json_new(JSON_NODE);
		  json_set_i(test1, 14);
		  json_set_i(test2, 35);
		  json_swap(test1, test2);
		  assertEquals(json_as_int(test1), 35);
		  assertEquals(json_as_int(test2), 14);
   
		  UnitTest::SetPrefix("Duplicate");
		  json_delete(test1);
		  test1 = json_duplicate(test2);
		  UNIT_TEST(assertNotEquals(((JSONNode*)test1) -> internal, ((JSONNode*)test2) -> internal);)
		  assertTrue(json_equal(test1, test2));
    
    
		  UnitTest::SetPrefix("Duplicate with children");
		  JSONNODE * node = json_new(JSON_NODE);
		  json_push_back(node, json_new_i(JSON_TEXT(""), 15));
		  json_push_back(node, json_new_f(JSON_TEXT(""), 27.4f));
		  json_push_back(node, json_new_b(JSON_TEXT(""), true));
		  
		  TEST_PARSING_ITSELF(node);
		  
		  JSONNODE * dup = json_duplicate(node);
		  assertEquals(json_size(dup), 3);
		  UNIT_TEST(assertNotEquals(((JSONNode*)node) -> internal, ((JSONNode*)dup) -> internal);)
		  assertEquals(json_type(dup), JSON_NODE);
		  
		  TEST_PARSING_ITSELF(node);
		  TEST_PARSING_ITSELF(dup);

		  assertEquals(json_as_int(json_at(dup, 0)), 15);
		  assertEquals(json_as_float(json_at(dup, 1)), 27.4f);
		  assertEquals(json_as_bool(json_at(dup, 2)), true);
		  assertTrue(json_equal(json_at(dup, 0), json_at(node, 0)));
		  assertTrue(json_equal(json_at(dup, 1), json_at(node, 1)));
		  assertTrue(json_equal(json_at(dup, 2), json_at(node, 2)));

    
		  TEST_PARSING_ITSELF(dup);
    
		  #ifdef JSON_ITERATORS
			 for(JSONNODE_ITERATOR it = json_begin(node), end = json_end(node), dup_it = json_begin(dup); 
				it != end; 
				++it, ++dup_it){
				assertTrue(json_equal(*it, *dup_it));
				UNIT_TEST(assertNotEquals(((JSONNode*)(*it)) -> internal, ((JSONNode*)(*dup_it)) -> internal);)
			 }
		  #endif
    
		  UnitTest::SetPrefix("Nullify");
		  json_nullify(test1);
		  assertEquals(json_type(test1), JSON_NULL);
		  json_char * res = json_name(test1);
		  assertCStringSame(res, JSON_TEXT(""));
		  json_free(res);
    
		  UnitTest::SetPrefix("Cast");
		  json_cast(test1, JSON_NULL);
		  json_set_i(test2, 1);
		  json_cast(test2, JSON_BOOL);
		  assertEquals(json_type(test1), JSON_NULL);
		  assertEquals(json_type(test2), JSON_BOOL);
		  assertEquals(json_as_bool(test2), true);
		  json_set_b(test2, true);
		  assertEquals(json_as_bool(test2), true);
    
		  json_cast(test2, JSON_NUMBER);
		  assertEquals(json_as_float(test2), 1.0f);
		  json_set_f(test2, 0.0f);
		  assertEquals(json_as_float(test2), 0.0f);
		  json_cast(test2, JSON_BOOL);
		  assertEquals(json_as_bool(test2), false);
    
    
		  UnitTest::SetPrefix("Merge");
		  json_set_a(test1, JSON_TEXT("hello"));
		  json_set_a(test2, JSON_TEXT("hello"));
		  UNIT_TEST(assertNotEquals(((JSONNode*)test1) -> internal, ((JSONNode*)test2) -> internal);)
		  assertTrue(json_equal(test1, test2));
		  json_merge(test1, test2);
		  UNIT_TEST(
			 #ifdef JSON_REF_COUNT
				  assertEquals(((JSONNode*)test1) -> internal, ((JSONNode*)test2) -> internal);
			 #else
				  assertNotEquals(((JSONNode*)test1) -> internal, ((JSONNode*)test2) -> internal);
			 #endif
		  )
    
		  json_cast(test1, JSON_NODE);
		  json_cast(test2, JSON_NODE);
		  assertEquals(json_type(test1), JSON_NODE);
		  assertEquals(json_type(test2), JSON_NODE);
		  json_push_back(test1, json_new_a(JSON_TEXT("hi"), JSON_TEXT("world")));
		  json_push_back(test2, json_new_a(JSON_TEXT("hi"), JSON_TEXT("world")));
		  
		  TEST_PARSING_ITSELF(test1);
		  TEST_PARSING_ITSELF(test2);
		  
		  json_merge(test1, test2);
		  UNIT_TEST(
			 #ifdef JSON_REF_COUNT
				  assertEquals(((JSONNode*)test1) -> internal, ((JSONNode*)test2) -> internal);
			 #else
				  assertNotEquals(((JSONNode*)test1) -> internal, ((JSONNode*)test2) -> internal);
			 #endif
				  )
    
		  TEST_PARSING_ITSELF(test1);
		  TEST_PARSING_ITSELF(test2);
    
    
		  json_delete(test1);
		  json_delete(test2);
		  json_delete(node);
		  json_delete(dup);
    #else
		  JSONNode test1;
		  JSONNode test2;
		  test1 = JSON_TEXT("hello");
		  test2 = JSON_TEXT("world");
		  test1.swap(test2);
		  assertEquals(test1, JSON_TEXT("world"));
		  assertEquals(test2, JSON_TEXT("hello"));
		  
		  UnitTest::SetPrefix("Duplicate");
		  test1 = test2.duplicate();
		  UNIT_TEST(assertNotEquals(test1.internal, test2.internal);)
		  assertEquals(test1, test2);
		  
		  UnitTest::SetPrefix("Duplicate with children");
		  JSONNode node = JSONNode(JSON_NODE);
		  node.push_back(JSONNode(JSON_TEXT(""), 15));
		  node.push_back(JSONNode(JSON_TEXT(""), JSON_TEXT("hello world")));
		  node.push_back(JSONNode(JSON_TEXT(""), true));
		  
		  TEST_PARSING_ITSELF(node);
		  
		  JSONNode dup = node.duplicate();
		  assertEquals(dup.size(), 3);
		  UNIT_TEST(assertNotEquals(node.internal, dup.internal);)
		  assertEquals(dup.type(), JSON_NODE);
		  
		  TEST_PARSING_ITSELF(node);
		  TEST_PARSING_ITSELF(dup);
		  
		  try {
			 assertEquals(dup.at(0), 15);
			 assertEquals(dup.at(1), JSON_TEXT("hello world"));
			 assertEquals(dup.at(2), true);
			 assertEquals(dup.at(0), node.at(0));
			 assertEquals(dup.at(1), node.at(1));
			 assertEquals(dup.at(2), node.at(2));
		  } catch (std::out_of_range){
			 FAIL("exception caught");
		  }
		  
		  TEST_PARSING_ITSELF(dup);
		  
		  #ifdef JSON_ITERATORS
			 for(JSONNode::iterator it = node.begin(), end = node.end(), dup_it = dup.begin(); 
				it != end; 
				++it, ++dup_it){
				assertEquals(*it, *dup_it);
				UNIT_TEST(assertNotEquals((*it).internal, (*dup_it).internal);)
			 }
		  #endif
		  
		  UnitTest::SetPrefix("Nullify");
		  test1.nullify();
		  assertEquals(test1.type(), JSON_NULL);
		  assertEquals(test1.name(), JSON_TEXT(""));
		  
		  UnitTest::SetPrefix("Cast");
		  test1.cast(JSON_NULL);
		  test2 = 1;
		  test2.cast(JSON_BOOL);
		  assertEquals(test1.type(), JSON_NULL);
		  assertEquals(test2.type(), JSON_BOOL);
		  assertEquals(test2, true);
		  test2 = true;
		  assertEquals(test2, true);
		  test2.cast(JSON_NUMBER);
		  assertEquals(test2, 1.0f);
		  test2 = 0.0f;
		  assertEquals(test2, 0.0f);
		  test2.cast(JSON_BOOL);
		  assertEquals(test2, false);
		  
		  UnitTest::SetPrefix("Merge");
		  test1 = JSON_TEXT("hello");
		  test2 = JSON_TEXT("hello");
		  UNIT_TEST(assertNotEquals(test1.internal, test2.internal);)
		  assertEquals(test1, test2);
		  test1.merge(test2);
		  UNIT_TEST(
			 #ifdef JSON_REF_COUNT
				  assertEquals(test1.internal, test2.internal);
			 #else
				  assertNotEquals(test1.internal, test2.internal);
			 #endif
		  )
		  
		  test1.cast(JSON_NODE);
		  test2.cast(JSON_NODE);
		  assertEquals(test1.type(), JSON_NODE);
		  assertEquals(test2.type(), JSON_NODE);
		  test1.push_back(JSONNode(JSON_TEXT("hi"), JSON_TEXT("world")));
		  test2.push_back(JSONNode(JSON_TEXT("hi"), JSON_TEXT("world")));
		  
		  TEST_PARSING_ITSELF(test1);
		  TEST_PARSING_ITSELF(test2);
		  
		  test1.merge(test2);
		  UNIT_TEST(
			 #ifdef JSON_REF_COUNT
				  assertEquals(test1.internal, test2.internal);
			 #else
				  assertNotEquals(test1.internal, test2.internal);
			 #endif
		  )
		  
		  TEST_PARSING_ITSELF(test1);
		  TEST_PARSING_ITSELF(test2);
    #endif
}
