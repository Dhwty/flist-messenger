#include "TestSuite.h"

#if defined(JSON_MEMORY_MANAGE) && defined(JSON_LIBRARY)
    #include "../Source/JSONMemory.h"
    extern auto_expand StringHandler; 
    extern auto_expand_node NodeHandler;
#endif

void TestSuite::TestNamespace(void){
    #ifdef JSON_LIBRARY
		  UnitTest::SetPrefix("Memory Manager");
		  #ifdef JSON_MEMORY_MANAGE
			 #define ASSERT_ZERO_ALLOCATIONS()\
				assertEquals(StringHandler.mymap.size(), 0);\
				assertEquals(NodeHandler.mymap.size(), 0)	   
			 ASSERT_ZERO_ALLOCATIONS();
			 JSONNODE * test = json_new(JSON_NODE);
			 json_set_a(test, JSON_TEXT("Hello"));
			 assertCStringSame(json_as_string(test), JSON_TEXT("Hello"));
			 test = json_new_f(JSON_TEXT("Hi"), 14.3);
			 assertCStringSame(json_name(test), JSON_TEXT("Hi"));
			 assertEquals(StringHandler.mymap.size(), 2);
			 assertEquals(NodeHandler.mymap.size(), 2);
			 json_delete(test);
			 assertEquals(NodeHandler.mymap.size(), 1);
			 json_delete_all();
			 assertEquals(NodeHandler.mymap.size(), 0);
			 json_free_all();
			 ASSERT_ZERO_ALLOCATIONS();
		  #else
			 #define ASSERT_ZERO_ALLOCATIONS() (void)0
		  #endif
		  UnitTest::SetPrefix("Stripper");
		  {
			 ASSERT_ZERO_ALLOCATIONS();
			 const json_char * json = JSON_TEXT("{\n\t\"hello\" : \"world\"\r\n}  ");
			 const json_char * stripped = JSON_TEXT("{\"hello\":\"world\"}");
			 json_char * res = json_strip_white_space(json);
			 assertCStringSame(res, stripped);
			 json_free(res);
			 ASSERT_ZERO_ALLOCATIONS();
		  }

		  {
			 ASSERT_ZERO_ALLOCATIONS();
			 const json_char * json = JSON_TEXT("/*comment*/{#comment\n\n\t\"hello\" ://comment\n \"world\"\r\n}  ");
			 const json_char * stripped = JSON_TEXT("{\"hello\":\"world\"}");
			 json_char * res = json_strip_white_space(json);
			 assertCStringSame(res, stripped);
			 json_free(res);
			 ASSERT_ZERO_ALLOCATIONS();
		  }
		  {
			 ASSERT_ZERO_ALLOCATIONS();
			 const json_char * json = JSON_TEXT("[\n\t\"hello world\" , \"hello mars\"\r\n]  ");
			 const json_char * stripped = JSON_TEXT("[\"hello world\",\"hello mars\"]");
			 json_char * res = json_strip_white_space(json);
			 assertCStringSame(res, stripped);
			 json_free(res);
			 ASSERT_ZERO_ALLOCATIONS();
		  }
		  {
			 ASSERT_ZERO_ALLOCATIONS();
			 const json_char * json = JSON_TEXT("  {\n\t\"hello\" : true\r\n}");
			 const json_char * stripped = JSON_TEXT("{\"hello\":true}");
			 json_char * res = json_strip_white_space(json);
			 assertCStringSame(res, stripped);
			 json_free(res);
			 ASSERT_ZERO_ALLOCATIONS();
		  }
		  {
			 ASSERT_ZERO_ALLOCATIONS();
			 const json_char * json = JSON_TEXT("  [\n\ttrue , false\r\n]");
			 const json_char * stripped = JSON_TEXT("[true,false]");
			 json_char * res = json_strip_white_space(json);
			 assertCStringSame(res, stripped);
			 json_free(res);
			 ASSERT_ZERO_ALLOCATIONS();
		  }
		  {
			 ASSERT_ZERO_ALLOCATIONS();
			 const json_char * json = JSON_TEXT("[true,false]");
			 const json_char * stripped = JSON_TEXT("[true,false]");
			 json_char * res = json_strip_white_space(json);
			 assertCStringSame(res, stripped);
			 json_free(res);
			 ASSERT_ZERO_ALLOCATIONS();
		  }
	   
		  #ifdef JSON_VALIDATE
			 UnitTest::SetPrefix("Validator");
			 JSONNODE * tester;
			 if (tester = json_validate(JSON_TEXT("[true,false]  "))){
				assertEquals(json_type(tester), JSON_ARRAY);
				assertEquals(json_size(tester), 2);
				UNIT_TEST(
						IF_FETCHABLE(assertTrue(((JSONNode*)tester) -> internal -> fetched);)
						IF_FETCHABLE(assertTrue(((JSONNode*)json_at(tester, 0)) -> internal -> fetched);)
						)
				TEST_PARSING_ITSELF(tester);
				json_delete(tester);
			 } else {
				FAIL("Exception caught in valid json 1");
			 }
		  
			 if (tester = json_validate(JSON_TEXT("  {\"hello\":\"world\"}"))){
				assertEquals(json_type(tester), JSON_NODE);
				UNIT_TEST(
						IF_FETCHABLE(assertTrue(((JSONNode*)tester) -> internal -> fetched);)
						IF_FETCHABLE(assertTrue(((JSONNode*)json_at(tester, 0)) -> internal -> fetched);)
						)
				assertEquals(json_size(tester), 1);
				TEST_PARSING_ITSELF(tester);
				json_delete(tester);
			 } else {
				FAIL("Exception caught in valid json 1");
			 }
	   
			 if (tester = json_validate(JSON_TEXT("  {\"hello\":null}"))){
				assertEquals(json_type(tester), JSON_NODE);
				assertEquals(json_size(tester), 1);
				assertEquals(json_type(json_at(tester, 0)), JSON_NULL);
				TEST_PARSING_ITSELF(tester);
				json_delete(tester);
			 } else {
				FAIL("Exception caught in valid json 3");
			 }
	   
		  
			 if(tester = json_validate(JSON_TEXT("  {\"hello\":}"))){
				assertEquals(json_type(tester), JSON_NODE);
				assertEquals(json_size(tester), 1);
				assertEquals(json_type(json_at(tester, 0)), JSON_NULL);
				TEST_PARSING_ITSELF(tester);
				json_delete(tester);
			 } else {
				FAIL("Exception caught in valid json 4");
			 }

			 if(tester = json_validate(JSON_TEXT("  {\"hello\":null, \"hi\" : \"Mars\"}"))){
				assertEquals(json_type(tester), JSON_NODE);
				assertEquals(json_size(tester), 2);
				assertEquals(json_type(json_at(tester, 0)), JSON_NULL);
				assertEquals(json_type(json_at(tester, 1)), JSON_STRING);
				json_char * res = json_as_string(json_at(tester, 1));
				assertCStringSame(res, JSON_TEXT("Mars"));
				json_free(res);
				TEST_PARSING_ITSELF(tester);
				json_delete(tester);
			 } else {
				FAIL("Exception caught in valid json 5");
			 }
		  
			 if(tester = json_validate(JSON_TEXT("  {\"hello\":, \"hi\" : \"Mars\"}"))){
				assertEquals(json_type(tester), JSON_NODE);
				assertEquals(json_size(tester), 2);
				assertEquals(json_type(json_at(tester, 0)), JSON_NULL);
				assertEquals(json_type(json_at(tester, 1)), JSON_STRING);
				json_char * res = json_as_string(json_at(tester, 1));
				assertCStringSame(res, JSON_TEXT("Mars"));
				json_free(res);
				TEST_PARSING_ITSELF(tester);
				json_delete(tester);
			 } else {
				FAIL("Exception caught in valid json 5");
			 }
		  
			 assertNull(json_validate(JSON_TEXT("{\"hello\":\"world\"")));
			 assertNull(json_validate(JSON_TEXT("\"hello\":\"world\"")));
			 assertNull(json_validate(JSON_TEXT("true,false]")));
			 assertNull(json_validate(JSON_TEXT("[true,false")));
			 assertNull(json_validate(JSON_TEXT("hello")));
			 assertNull(json_validate(JSON_TEXT("")));
			 #ifdef JSON_SAFE
				assertNull(json_validate(JSON_TEXT("  {\"hello\":world\"}")));
			 #endif
		  #endif
    #else
		  UnitTest::SetPrefix("Stripper");
		  {
			 json_string json = JSON_TEXT("{\n\t\"hello\" : \"world\"\r\n}  ");
			 json_string stripped = JSON_TEXT("{\"hello\":\"world\"}");
			 assertEquals(libJSON::strip_white_space(json), stripped);
		  }
		  {
			 json_string json = JSON_TEXT("/*comment*/{#comment\n\n\t\"hello\" ://comment\n \"world\"\r\n}  ");
			 json_string stripped = JSON_TEXT("{\"hello\":\"world\"}");
			 assertEquals(libJSON::strip_white_space(json), stripped);
		  }
		  {
			 json_string json = JSON_TEXT("[\n\t\"hello world\" , \"hello mars\"\r\n]  ");
			 json_string stripped = JSON_TEXT("[\"hello world\",\"hello mars\"]");
			 assertEquals(libJSON::strip_white_space(json), stripped);
		  }
		  {
			 json_string json = JSON_TEXT("  {\n\t\"hello\" : true\r\n}");
			 json_string stripped = JSON_TEXT("{\"hello\":true}");
			 assertEquals(libJSON::strip_white_space(json), stripped);
		  }
		  {
			 json_string json = JSON_TEXT("  [\n\ttrue , false\r\n]");
			 json_string stripped = JSON_TEXT("[true,false]");
			 assertEquals(libJSON::strip_white_space(json), stripped);
		  }
		  {
			 json_string json = JSON_TEXT("[true,false]");
			 json_string stripped = JSON_TEXT("[true,false]");
			 assertEquals(libJSON::strip_white_space(json), stripped);
		  }
		  
		  #ifdef JSON_VALIDATE
			 UnitTest::SetPrefix("Validator");
			 JSONNode tester;
			 try {
			 tester = libJSON::validate(JSON_TEXT("[true,false]  "));
				assertEquals(tester.type(), JSON_ARRAY);
				UNIT_TEST(
				    IF_FETCHABLE(assertTrue(tester.internal -> fetched);)
				    IF_FETCHABLE(assertTrue(tester[0].internal -> fetched);)
				)
				assertEquals(tester.size(), 2);
				TEST_PARSING_ITSELF(tester);
			 } catch (std::invalid_argument){
				FAIL("Exception caught in valid json 1");
			 }
		  
			 try {
				tester = libJSON::validate(JSON_TEXT("  {\"hello\":\"world\"}"));
				assertEquals(tester.type(), JSON_NODE);
				UNIT_TEST(
				    IF_FETCHABLE(assertTrue(tester.internal -> fetched);)
				    IF_FETCHABLE(assertTrue(tester[0].internal -> fetched);)
				)
				assertEquals(tester.size(), 1);
				TEST_PARSING_ITSELF(tester);
			 } catch (std::invalid_argument){
				FAIL("Exception caught in valid json 2");
			 }
		  
			 try {
				tester = libJSON::validate(JSON_TEXT("  {\"hello\":null}"));
				assertEquals(tester.type(), JSON_NODE);
				assertEquals(tester.size(), 1);
				assertEquals(tester[0].type(), JSON_NULL);
				TEST_PARSING_ITSELF(tester);
			 } catch (std::invalid_argument){
				FAIL("Exception caught in valid json 3");
			 }
			 
			 try {
				tester = libJSON::validate(JSON_TEXT("  {\"hello\":}"));
				assertEquals(tester.type(), JSON_NODE);
				assertEquals(tester.size(), 1);
				assertEquals(tester[0].type(), JSON_NULL);
				TEST_PARSING_ITSELF(tester);
			 } catch (std::invalid_argument){
				FAIL("Exception caught in valid json 4");
			 }
			 
			 try {
				tester = libJSON::validate(JSON_TEXT("  {\"hello\":null, \"hi\" : \"Mars\"}"));
				assertEquals(tester.type(), JSON_NODE);
				assertEquals(tester.size(), 2);
				assertEquals(tester[0].type(), JSON_NULL);
				assertEquals(tester[1].type(), JSON_STRING);
				assertEquals(tester[1], JSON_TEXT("Mars"));
				TEST_PARSING_ITSELF(tester);
			 } catch (std::invalid_argument){
				FAIL("Exception caught in valid json 5");
			 }
			 
			 try {
				tester = libJSON::validate(JSON_TEXT("  {\"hello\":, \"hi\" : \"Mars\"}"));
				assertEquals(tester.type(), JSON_NODE);
				assertEquals(tester.size(), 2);
				assertEquals(tester[0].type(), JSON_NULL);
				assertEquals(tester[1].type(), JSON_STRING);
				assertEquals(tester[1], JSON_TEXT("Mars"));
				TEST_PARSING_ITSELF(tester);
			 } catch (std::invalid_argument){
				FAIL("Exception caught in valid json 6");
			 }
			 
			 assertException(libJSON::validate(JSON_TEXT("{\"hello\":\"world\"")), std::invalid_argument);
			 assertException(libJSON::validate(JSON_TEXT("\"hello\":\"world\"")), std::invalid_argument);
			 assertException(libJSON::validate(JSON_TEXT("true,false]")), std::invalid_argument);
			 assertException(libJSON::validate(JSON_TEXT("[true,false")), std::invalid_argument);
			 assertException(libJSON::validate(JSON_TEXT("hello")), std::invalid_argument);
			 assertException(libJSON::validate(JSON_TEXT("")), std::invalid_argument);
			 #ifdef JSON_SAFE
				assertException(libJSON::validate(JSON_TEXT("  {\"hello\":world\"}")), std::invalid_argument);
			 #endif
		  #else
			 JSONNode tester;
		  #endif
		  
		  UnitTest::SetPrefix("Parse");
		  tester = libJSON::parse(JSON_TEXT("\r\n{\"hello\":\"world\"}"));
		  assertEquals(tester.type(), JSON_NODE);
		  UNIT_TEST(
				  IF_FETCHABLE(
							assertFalse(tester.internal -> fetched);
							tester.preparse();
							assertTrue(tester.internal -> fetched);
							assertTrue(tester[0].internal -> fetched);
							)
				  )
		  assertEquals(tester.size(), 1);
		  assertEquals(tester[0].name(), JSON_TEXT("hello"));
		  assertEquals(tester[0], JSON_TEXT("world"));
		  UNIT_TEST(IF_FETCHABLE(assertTrue(tester.internal -> fetched);))
		  #ifdef JSON_SAFE
			 assertException(libJSON::parse(JSON_TEXT("{\"hello\":\"world\"")), std::invalid_argument);
		  #endif
		  assertException(libJSON::parse(JSON_TEXT("\"hello\":\"world\"")), std::invalid_argument);
		  tester = libJSON::parse(JSON_TEXT(" [true, false]\r\n"));
		  assertEquals(tester.type(), JSON_ARRAY);
		  UNIT_TEST(IF_FETCHABLE(assertFalse(tester.internal -> fetched);))
		  assertEquals(tester.size(), 2);
		  UNIT_TEST(IF_FETCHABLE(assertTrue(tester.internal -> fetched);))
		  assertException(libJSON::parse(JSON_TEXT("true,false]")), std::invalid_argument);
		  #ifdef JSON_SAFE
			 assertException(libJSON::parse(JSON_TEXT("[true,false")), std::invalid_argument);
		  #endif
		  assertException(libJSON::parse(JSON_TEXT("hello")), std::invalid_argument);
		  assertException(libJSON::parse(JSON_TEXT("")), std::invalid_argument);
		  TEST_PARSING_ITSELF(tester);
		  
		  tester = libJSON::parse(JSON_TEXT(" [\"hello\", \"world\", \"mars\"]\r\n"));
		  assertEquals(tester.type(), JSON_ARRAY);
		  assertEquals(tester.size(), 3);
		  assertEquals(tester[0], JSON_TEXT("hello"));
		  assertEquals(tester[1], JSON_TEXT("world"));
		  assertEquals(tester[2], JSON_TEXT("mars"));
		  TEST_PARSING_ITSELF(tester);
		  
		  tester = libJSON::parse(JSON_TEXT("{\"\":{},\"\":2}"));
		  assertEquals(tester.type(), JSON_NODE);
		  assertEquals(tester.size(), 2);
		  assertEquals(tester[0].type(), JSON_NODE);
		  assertTrue(tester[0].empty());
		  assertEquals(tester[1].type(), JSON_NUMBER);
		  assertEquals(tester[1], 2);
		  assertEquals(tester, libJSON::parse(JSON_TEXT("{\"\":{},\"\":2}")));
		  TEST_PARSING_ITSELF(tester);
		  
		  tester = libJSON::parse(JSON_TEXT("\r\n{\"hello\":\"world\", \"hi\":\"mars\"}"));
		  assertEquals(tester.type(), JSON_NODE);
		  UNIT_TEST(IF_FETCHABLE(assertFalse(tester.internal -> fetched);))
		  assertEquals(tester.size(), 2);
		  assertEquals(tester[0].name(), JSON_TEXT("hello"));
		  assertEquals(tester[0], JSON_TEXT("world"));
		  assertEquals(tester[1].name(), JSON_TEXT("hi"));
		  assertEquals(tester[1], JSON_TEXT("mars"));
		  TEST_PARSING_ITSELF(tester);
		  
		  tester = libJSON::parse(JSON_TEXT("\r\n{\"hello\":\"world\", \"hi\":\"mars\", \"and\":\"pluto\"}"));
		  assertEquals(tester.type(), JSON_NODE);
		  UNIT_TEST(IF_FETCHABLE(assertFalse(tester.internal -> fetched);))
		  assertEquals(tester.size(), 3);
		  assertEquals(tester[0].name(), JSON_TEXT("hello"));
		  assertEquals(tester[0], JSON_TEXT("world"));
		  assertEquals(tester[1].name(), JSON_TEXT("hi"));
		  assertEquals(tester[1], JSON_TEXT("mars"));
		  assertEquals(tester[2].name(), JSON_TEXT("and"));
		  assertEquals(tester[2], JSON_TEXT("pluto"));
		  TEST_PARSING_ITSELF(tester);
    #endif
}
