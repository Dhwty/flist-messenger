#include "TestSuite.h"
#include "../Source/NumberToString.h"
#include "../Source/JSON_Base64.h"
#include "../Source/JSONWorker.h"
#include <iostream>
void TestSuite::TestConverters(void){
    UnitTest::SetPrefix("Converters");

    assertEquals(sizeof(char), 1);
    assertEquals(NumberToString::_itoa<char>((char)127), JSON_TEXT("127"));
    assertEquals(NumberToString::_itoa<char>((char)15), JSON_TEXT("15"));
    assertEquals(NumberToString::_itoa<char>((char)0), JSON_TEXT("0"));
    assertEquals(NumberToString::_itoa<char>((char)-15), JSON_TEXT("-15"));
    assertEquals(NumberToString::_itoa<char>((char)-127), JSON_TEXT("-127"));
    
    assertEquals(sizeof(short), 2);
    assertEquals(NumberToString::_itoa<short>((short)32767), JSON_TEXT("32767"));
    assertEquals(NumberToString::_itoa<short>((short)15), JSON_TEXT("15"));
    assertEquals(NumberToString::_itoa<short>((short)0), JSON_TEXT("0"));
    assertEquals(NumberToString::_itoa<short>((short)-15), JSON_TEXT("-15"));
    assertEquals(NumberToString::_itoa<short>((short)-32767), JSON_TEXT("-32767"));
   
    assertEquals(sizeof(int), 4);
    assertEquals(NumberToString::_itoa<int>(2147483647), JSON_TEXT("2147483647"));
    assertEquals(NumberToString::_itoa<int>(15), JSON_TEXT("15"));
    assertEquals(NumberToString::_itoa<int>(0), JSON_TEXT("0"));
    assertEquals(NumberToString::_itoa<int>(-15), JSON_TEXT("-15"));
    assertEquals(NumberToString::_itoa<int>(-2147483647), JSON_TEXT("-2147483647"));
    
    if(sizeof(long) >= 8){
	   assertEquals(NumberToString::_itoa<long>(9223372036854775807L), JSON_TEXT("9223372036854775807"));
        assertEquals(NumberToString::_itoa<long>(-9223372036854775807L), JSON_TEXT("-9223372036854775807"));
	   #ifndef JSON_LIBRARY
		  assertEquals(NumberToString::_uitoa<unsigned long>(18446744073709551615UL), JSON_TEXT("18446744073709551615"));
	   #endif
    }  
    assertEquals(NumberToString::_itoa<long>(15), JSON_TEXT("15"));
    assertEquals(NumberToString::_itoa<long>(0), JSON_TEXT("0"));
    assertEquals(NumberToString::_itoa<long>(-15), JSON_TEXT("-15"));
   
    #ifndef JSON_LIBRARY
	   assertEquals(NumberToString::_uitoa<unsigned char>(255), JSON_TEXT("255"));
	   assertEquals(NumberToString::_uitoa<unsigned char>(15), JSON_TEXT("15"));
	   assertEquals(NumberToString::_uitoa<unsigned char>(0), JSON_TEXT("0"));
	   
	   assertEquals(NumberToString::_uitoa<unsigned short>(65535), JSON_TEXT("65535"));
	   assertEquals(NumberToString::_uitoa<unsigned short>(15), JSON_TEXT("15"));
	   assertEquals(NumberToString::_uitoa<unsigned short>(0), JSON_TEXT("0"));
	   
	   assertEquals(NumberToString::_uitoa<unsigned int>(4294967295u), JSON_TEXT("4294967295"));
	   assertEquals(NumberToString::_uitoa<unsigned int>(15), JSON_TEXT("15"));
	   assertEquals(NumberToString::_uitoa<unsigned int>(0), JSON_TEXT("0"));
	   
	   assertEquals(NumberToString::_uitoa<unsigned long>(15), JSON_TEXT("15"));
	   assertEquals(NumberToString::_uitoa<unsigned long>(0), JSON_TEXT("0"));
    #endif
    
    assertEquals(NumberToString::_ftoa<float>(1.0f), JSON_TEXT("1"));
    assertEquals(NumberToString::_ftoa<float>(1.002f), JSON_TEXT("1.002"));
    assertEquals(NumberToString::_ftoa<float>(10.0f), JSON_TEXT("10"));
    assertEquals(NumberToString::_ftoa<float>(-1.0f), JSON_TEXT("-1"));
    assertEquals(NumberToString::_ftoa<float>(-1.002f), JSON_TEXT("-1.002"));
    assertEquals(NumberToString::_ftoa<float>(-10.0f), JSON_TEXT("-10"));
    assertEquals(NumberToString::_ftoa<float>(0.0f), JSON_TEXT("0"));
    
    assertEquals(NumberToString::_ftoa<double>(1.0f), JSON_TEXT("1"));
    assertEquals(NumberToString::_ftoa<double>(1.002f), JSON_TEXT("1.002"));
    assertEquals(NumberToString::_ftoa<double>(10.0f), JSON_TEXT("10"));
    assertEquals(NumberToString::_ftoa<double>(-1.0f), JSON_TEXT("-1"));
    assertEquals(NumberToString::_ftoa<double>(-1.002f), JSON_TEXT("-1.002"));
    assertEquals(NumberToString::_ftoa<double>(-10.0f), JSON_TEXT("-10"));
    assertEquals(NumberToString::_ftoa<double>(0.0f), JSON_TEXT("0"));
    
    assertTrue(NumberToString::areEqual(1.1, 1.1));
    assertTrue(NumberToString::areEqual(1.000000001, 1.0));
    assertTrue(NumberToString::areEqual(1.0, 1.000000001));
    assertFalse(NumberToString::areEqual(1.0, 1.0001));
    assertFalse(NumberToString::areEqual(1.0001, 1.0));
    
    #ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	   UNIT_TEST(
		    UnitTest::SetPrefix("Checking case-insensitive");
		    assertTrue(internalJSONNode::AreEqualNoCase(JSON_TEXT("hello"), JSON_TEXT("HeLLo")));
		    assertTrue(internalJSONNode::AreEqualNoCase(JSON_TEXT("hell5o"), JSON_TEXT("HELL5O")));
		    assertTrue(internalJSONNode::AreEqualNoCase(JSON_TEXT("HeLLo"), JSON_TEXT("hello")));
		    assertTrue(internalJSONNode::AreEqualNoCase(JSON_TEXT("HELL5O"), JSON_TEXT("hell5o")));
		    
		    assertFalse(internalJSONNode::AreEqualNoCase(JSON_TEXT("hello"), JSON_TEXT("Hello ")));
		    assertFalse(internalJSONNode::AreEqualNoCase(JSON_TEXT("hello"), JSON_TEXT("hi")));
		    assertFalse(internalJSONNode::AreEqualNoCase(JSON_TEXT("hello"), JSON_TEXT("55555")));
		    assertFalse(internalJSONNode::AreEqualNoCase(JSON_TEXT("hello"), JSON_TEXT("jonny")));
		    )
    #endif
}

#ifdef JSON_BINARY
    void TestSuite::TestBase64(void){
	   UnitTest::SetPrefix("Base 64");
	   assertEquals(JSONBase64::json_decode64(JSONBase64::json_encode64((unsigned char *)"", 0)), "");
	   assertEquals(JSONBase64::json_decode64(JSONBase64::json_encode64((unsigned char *)"A", 1)), "A");
	   assertEquals(JSONBase64::json_decode64(JSONBase64::json_encode64((unsigned char *)"AB", 2)), "AB");
	   assertEquals(JSONBase64::json_decode64(JSONBase64::json_encode64((unsigned char *)"ABC", 3)), "ABC");
	   assertEquals(JSONBase64::json_decode64(JSONBase64::json_encode64((unsigned char *)"ABCD", 4)), "ABCD");
	   #ifdef JSON_SAFE
		  assertEquals(JSONBase64::json_decode64(JSON_TEXT("123!abc")), "");
		  assertEquals(JSONBase64::json_decode64(JSON_TEXT("123=abc")), "");
		  assertEquals(JSONBase64::json_decode64(JSON_TEXT("123abc===")), "");
	   #endif
    }
#endif
