#ifndef TESTSUITE_H
#define TESTSUITE_H

#include "UnitTest.h"
#include "../JSONOptions.h"
#include "../libJSON.h"

/*
    This class tests libJSON's internal working and it's 
    C++ interface
*/

#ifdef JSON_PREPARSE
    #define IF_FETCHABLE(code)
#else
    #define IF_FETCHABLE(code) code
#endif


#ifdef JSON_LIBRARY
    void TEST_PARSING_ITSELF(JSONNODE * x);
#else
    #ifdef JSON_WRITER
	   #define TEST_PARSING_ITSELF(x)\
		  assertEquals(libJSON::parse(x.write()), x);\
		  assertEquals(libJSON::parse(x.write_formatted()), x);\
		  assertEquals(libJSON::strip_white_space(x.write_formatted()), x.write());\
		  assertEquals(x, x.duplicate())
    #else
	   #define TEST_PARSING_ITSELF(x) assertEquals(x, x.duplicate())
    #endif
#endif

#ifdef JSON_UNIT_TEST
    #define UNIT_TEST(x) x
#else
    #define UNIT_TEST(x)
#endif

#ifdef JSON_UNICODE
    #define assertCStringSame(a, b) assertCStringEqualsW(a, b)
#else
     #define assertCStringSame(a, b) assertCStringEquals(a, b)
#endif

class TestSuite {
public:
    static void TestSelf(void);
    static void TestConverters(void);
#ifdef JSON_BINARY
    static void TestBase64(void);
#endif
    static void TestReferenceCounting(void);
    static void TestConstructors(void);
    static void TestAssigning(void);
    static void TestEquality(void);
    static void TestInequality(void);
    static void TestChildren(void);
    static void TestFunctions(void);
    static void TestIterators(void);
    static void TestInspectors(void);
    static void TestNamespace(void);
#ifdef JSON_WRITER
    static void TestWriter(void);
#endif
#ifdef JSON_COMMENTS
    static void TestComments(void);
#endif
#ifdef JSON_MUTEX_CALLBACKS
    static void TestMutex(void);
    static void TestThreading(void);
#endif
    static void TestFinal(void);
};

#endif

