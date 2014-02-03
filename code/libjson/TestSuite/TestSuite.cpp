#include "TestSuite.h"


#ifndef JSON_STDERROR
    #ifdef JSON_DEBUG
	   #ifdef JSON_LIBRARY
	   static void callback(const json_char * msg_c){
		  json_string msg(msg_c);
	   #else
	   static void callback(const json_string & msg){
	   #endif
		  #ifdef JSON_UNICODE
			 const std::string res = std::string(msg.begin(), msg.end());
			 echo(res);
		  #else
			 echo(msg);
		  #endif
	   }
    #endif
#endif

#ifdef JSON_LIBRARY
    #ifdef JSON_WRITER
	   void TEST_PARSING_ITSELF(JSONNODE * x){
		  if (json_char * _res = json_write(x)){
			 if(JSONNODE * _temp = json_parse(_res)){
				assertTrue(json_equal(_temp, x));
				json_delete(_temp);
			 } else {
				FAIL("parsing itself failed parsing write");
			 }
			 json_free(_res);
		  } else {
			 FAIL("parsing itself failed writing");
		  }
		  if (json_char * _res = json_write_formatted(x)){
			 if(JSONNODE * _tempf = json_parse(_res)){
				assertTrue(json_equal(_tempf, x));
				json_delete(_tempf);
			 } else {
				FAIL("parsing itself failed parsing write formatted");
			 }
			 
			 if (json_char * unform = json_write(x)){
				if (json_char * stripped = json_strip_white_space(unform)){
				    assertCStringSame(stripped, unform);
				    json_free(stripped);
				}
				json_free(unform);
			 } else {
				FAIL("parsing itself failed writing");
			 }
			 
			 json_free(_res);
		  } else {
			 FAIL("parsing itself failed writing formatted");
		  }
		  if(JSONNODE * _dup = json_duplicate(x)){
			 assertTrue(json_equal(_dup, x));
			 json_delete(_dup);
		  } else {
			 FAIL("parsing itself failed dup");
		  }
	   }
    #else
		  void TEST_PARSING_ITSELF(JSONNODE * x){
			 if(JSONNODE * _dup = json_duplicate(x)){
				assertTrue(json_equal(_dup, x));
				json_delete(_dup);
			 } else {
				FAIL("parsing itself failed dup");
			 }
		  }
    #endif
#endif
		  
void TestSuite::TestSelf(void){
    UnitTest::SetPrefix("Self Test");
    #ifndef JSON_STDERROR
	   #ifdef JSON_DEBUG
		  #ifdef JSON_LIBRARY
			 json_register_debug_callback(callback);
		  #else
			 libJSON::register_debug_callback(callback);
		  #endif
	   #endif
    #endif
    assertUnitTest();   
    
    #if defined(JSON_SAFE) && ! defined(JSON_LIBRARY)
	   bool temp = false;
	   JSON_ASSERT_SAFE(true, JSON_TEXT(""), temp = true;);
	   assertFalse(temp);
	   JSON_ASSERT_SAFE(false, JSON_TEXT(""), temp = true;);
	   assertTrue(temp);
    
	   temp = false;
	   JSON_FAIL_SAFE(JSON_TEXT(""), temp = true;);
	   assertTrue(temp);
    #endif
    
    echo("If this fails, then edit JSON_INDEX_TYPE in JSONOptions.h");
    assertLessThanEqualTo(sizeof(json_index_t), sizeof(void*)); 
}

//makes sure that libjson didn't leak memory somewhere
void TestSuite::TestFinal(void){
    UnitTest::SetPrefix("Memory Leak");
    UNIT_TEST(
	   echo("Node allocations: " << JSONNode::getNodeAllocationCount());
	   echo("Node deallocations: " << JSONNode::getNodeDeallocationCount());
	   assertEquals(JSONNode::getNodeAllocationCount(), JSONNode::getNodeDeallocationCount());
	   
	   echo("internal allocations: " << JSONNode::getInternalAllocationCount());
	   echo("internal deallocations: " << JSONNode::getInternalDeallocationCount());
	   assertEquals(JSONNode::getInternalAllocationCount(), JSONNode::getInternalDeallocationCount());
    )
}
