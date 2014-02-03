#include <iostream>
#include <cstdlib> //for malloc, realloc, and free
#include "TestSuite.h"
#include "../libJSON.h"

void DoTests(void){
    TestSuite::TestConverters();
    #ifdef JSON_BINARY
	   TestSuite::TestBase64();
    #endif
    TestSuite::TestReferenceCounting();
    TestSuite::TestConstructors();
    TestSuite::TestAssigning();
    TestSuite::TestEquality();
    TestSuite::TestInequality();
    TestSuite::TestChildren();
    TestSuite::TestFunctions();
    TestSuite::TestIterators();
    TestSuite::TestInspectors();
    TestSuite::TestNamespace();
    #ifdef JSON_WRITER
	   TestSuite::TestWriter();
    #endif
    #ifdef JSON_COMMENTS
	   TestSuite::TestComments();
    #endif
    #ifdef JSON_MUTEX_CALLBACKS
	   TestSuite::TestMutex();
	   TestSuite::TestThreading();
    #endif
    TestSuite::TestFinal();
}

#ifdef JSON_MEMORY_CALLBACKS
    int mallocs = 0;
    int frees = 0;
    #ifdef JSON_LIBRARY
	   void * testmal(unsigned long siz){ ++mallocs; return malloc(siz); }
	   void * testreal(void * ptr, unsigned long siz){ return realloc(ptr, siz); }
    #else
	   void * testmal(size_t siz){ ++mallocs; return malloc(siz); }
	   void * testreal(void * ptr, size_t siz){ return realloc(ptr, siz); }
    #endif
    void testfree(void * ptr){ ++frees; free(ptr); }

    void doMemTests(void){
	   #ifdef JSON_LIBRARY
		  json_register_memory_callbacks(testmal, testreal, testfree);
	   #else
		  libJSON::register_memory_callbacks(testmal, testreal, testfree);
	   #endif
	   DoTests();
	   echo("mallocs: " << mallocs);
	   echo("frees: " << frees);
	   assertEquals(mallocs, frees);
    }
#endif

int main () {
    UnitTest::StartTime();
    TestSuite::TestSelf();

    DoTests();
    
    #ifdef JSON_MEMORY_CALLBACKS
	   doMemTests();
    #endif
    
    UnitTest::SaveTo("out.html");
    
    return 0;
}
