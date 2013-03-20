#include "TestSuite.h"



#ifdef JSON_MUTEX_CALLBACKS
    int testMutex = 0;
    bool doassert = true;
    int managerlock;
    static void lock(void * mutex){
	   if (mutex == &managerlock) return;
	   if (doassert) assertEquals(mutex, &testMutex);
	   if (mutex != &testMutex) return;  //to avoid access violations to tests fail, but don't crash
	   ++(*((int*)mutex));
    }
    static void unlock(void * mutex){
	   if (mutex == &managerlock) return;
	   if (doassert) assertEquals(mutex, &testMutex);
	   if (mutex != &testMutex) return;  //to avoid access violations to tests fail, but don't crash
	   --(*((int*)mutex));
    }

    void * currentMutexTest = 0;

    #ifdef JSON_MUTEX_MANAGE
	   #include <map>
	   extern std::map<void *, unsigned int> mutex_manager;

	   static void destroy(void * mutex){
		  assertEquals(mutex, currentMutexTest);
		  assertEquals(*((int*)mutex), 0);
	   }
    #endif

    void TestSuite::TestMutex(void){
	   UnitTest::SetPrefix("Mutex");
	   
	   #ifdef JSON_LIBRARY
		  
		  
		  #ifdef JSON_MUTEX_MANAGE
			 json_register_mutex_callbacks(lock, unlock, destroy, &managerlock);
		  #else
			 json_register_mutex_callbacks(lock, unlock, &managerlock);
		  #endif
		  
		  currentMutexTest = &testMutex;
		  {
			 JSONNODE * test1 = json_new(JSON_NODE);;
			 UNIT_TEST(assertNull(((JSONNode*)test1) -> internal -> mylock);)
			 JSONNODE * test2 = json_copy(test1);
			 UNIT_TEST(assertNull(((JSONNode*)test1) -> internal -> mylock);)
			 json_set_mutex(test2, &testMutex);
			 UNIT_TEST(assertEquals(((JSONNode*)test2) -> internal -> mylock, &testMutex);)
			 UNIT_TEST(assertNull(((JSONNode*)test1) -> internal -> mylock);)
			 
			 JSONNODE * test3 = json_copy(test2);
			 UNIT_TEST(assertEquals(((JSONNode*)test3) -> internal -> mylock, &testMutex);)
			 UNIT_TEST(assertEquals(((JSONNode*)test2) -> internal -> mylock, &testMutex);)
			 json_set_a(test3, JSON_TEXT("Hello World"));
			 UNIT_TEST(assertEquals(((JSONNode*)test3) -> internal -> mylock, &testMutex);)
			 
			 json_cast(test3, JSON_NODE);
			 UNIT_TEST(assertEquals(((JSONNode*)test3) -> internal -> mylock, &testMutex);)
			 
			 JSONNODE * tree = json_new(JSON_NODE);
			 json_push_back(tree, json_new_a(JSON_TEXT("Hello"), JSON_TEXT("world")));
			 json_push_back(tree, json_new_a(JSON_TEXT("Hello"), JSON_TEXT("Mars")));
			 json_push_back(tree, json_new_a(JSON_TEXT("Hello"), JSON_TEXT("USA")));
			 json_push_back(test3, json_copy(tree));
			 UNIT_TEST(
					 assertEquals(((JSONNode*)test3) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(test3, 0)) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(json_at(test3, 0), 0)) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(json_at(test3, 0), 1)) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(json_at(test3, 0), 2)) -> internal -> mylock, &testMutex);
					 )
			
			 json_clear(test3);
			 json_set_mutex(test3, 0);
			 assertEquals(json_size(test3), 0);
			 assertEquals(json_size(tree), 3);
			 UNIT_TEST(
					 assertNull(((JSONNode*)tree) -> internal -> mylock);
					 assertNull(((JSONNode*)json_at(tree, 0)) -> internal -> mylock);
					 assertNull(((JSONNode*)json_at(tree, 1)) -> internal -> mylock);
					 assertNull(((JSONNode*)json_at(tree, 2)) -> internal -> mylock);
					 )
			 json_set_mutex(tree, &testMutex);
			 UNIT_TEST(
					 assertEquals(((JSONNode*)tree) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(tree, 0)) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(tree, 1)) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(tree, 2)) -> internal -> mylock, &testMutex);
					 )
			 json_push_back(test3, tree);
			 UNIT_TEST(
					 assertNull(((JSONNode*)test3) -> internal -> mylock);
					 assertEquals(((JSONNode*)json_at(test3, 0)) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(json_at(test3, 0), 0)) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(json_at(test3, 0), 1)) -> internal -> mylock, &testMutex);
					 assertEquals(((JSONNode*)json_at(json_at(test3, 0), 2)) -> internal -> mylock, &testMutex);
					 )		  
			 assertEquals(testMutex, 0);
			 
			 #ifdef JSON_MUTEX_MANAGE
			 {
				JSONNODE * deleteTest = json_new(JSON_NODE);
				int i = 0;
				currentMutexTest = &i;
				json_set_mutex(deleteTest, &i);
				std::map<void *, unsigned int>::iterator it = mutex_manager.find((void*)&i);
				assertEquals(mutex_manager.size(), 2);
				assertNotEquals(it, mutex_manager.end());
				assertEquals(it -> first, (void*)&i);
				assertEquals(it -> second, 1); 
				
				json_set_mutex(deleteTest, &testMutex);
				currentMutexTest = &testMutex;
				json_delete(deleteTest);
			 }
			 #endif
			 
			 json_delete(test1);
			 json_delete(test2);
			 json_delete(test3);
		  }
		  #ifdef JSON_MUTEX_MANAGE
			 std::map<void *, unsigned int>::iterator it = mutex_manager.find((void*)&testMutex);
			 assertEquals(mutex_manager.size(), 0);
			 assertEquals(it, mutex_manager.end());
		  #endif

		  
	   
	   
	   
	   #else
		  #ifdef JSON_MUTEX_MANAGE
			 libJSON::register_mutex_callbacks(lock, unlock, destroy, &managerlock);
		  #else
			 libJSON::register_mutex_callbacks(lock, unlock, &managerlock);
		  #endif
		  
		  currentMutexTest = &testMutex;
		  {
			 JSONNode test1;
			 UNIT_TEST(assertNull(test1.internal -> mylock);)
			 JSONNode test2 = JSONNode(test1);
			 UNIT_TEST(assertNull(test1.internal -> mylock);)
			 test2.set_mutex(&testMutex);
			 UNIT_TEST(assertEquals(test2.internal -> mylock, &testMutex);)
			 UNIT_TEST(assertNull(test1.internal -> mylock);)
			 
			 JSONNode test3 = test2;
			 UNIT_TEST(assertEquals(test3.internal -> mylock, &testMutex);)
			 UNIT_TEST(assertEquals(test2.internal -> mylock, &testMutex);)
			 test3 = JSON_TEXT("Hello World");
			 UNIT_TEST(assertEquals(test3.internal -> mylock, &testMutex);)
			 
			 test3.cast(JSON_NODE);
			 UNIT_TEST(assertEquals(test3.internal -> mylock, &testMutex);)
			 JSONNode tree = JSONNode(JSON_NODE);
			 tree.push_back(JSONNode(JSON_TEXT("Hello"), JSON_TEXT("world")));
			 tree.push_back(JSONNode(JSON_TEXT("Hello"), JSON_TEXT("Mars")));
			 tree.push_back(JSONNode(JSON_TEXT("Hello"), JSON_TEXT("USA")));
			 test3.push_back(tree);
			 UNIT_TEST(
			   assertEquals(test3.internal -> mylock, &testMutex);
			   assertEquals(test3[0].internal -> mylock, &testMutex);
			   assertEquals(test3[0][0].internal -> mylock, &testMutex);
			   assertEquals(test3[0][1].internal -> mylock, &testMutex);
			   assertEquals(test3[0][2].internal -> mylock, &testMutex);
			 )
    
			 
			 test3.clear();
			 test3.set_mutex(0);
			 assertEquals(test3.size(), 0);
			 assertEquals(tree.size(), 3);
			 UNIT_TEST(
				  assertNull(tree.internal -> mylock);
				  assertNull(tree[0].internal -> mylock);
				  assertNull(tree[1].internal -> mylock);
				  assertNull(tree[2].internal -> mylock);
				)
			 tree.set_mutex(&testMutex);
			 UNIT_TEST(
				  assertEquals(tree.internal -> mylock, &testMutex);
				  assertEquals(tree[0].internal -> mylock, &testMutex);
				  assertEquals(tree[1].internal -> mylock, &testMutex);
				  assertEquals(tree[2].internal -> mylock, &testMutex);
				)
			 test3.push_back(tree);
			 UNIT_TEST(
				  assertNull(test3.internal -> mylock);
				  assertEquals(test3[0].internal -> mylock, &testMutex);
				  assertEquals(test3[0][0].internal -> mylock, &testMutex);
				  assertEquals(test3[0][1].internal -> mylock, &testMutex);
				  assertEquals(test3[0][2].internal -> mylock, &testMutex);
				)
			 #ifndef JSON_SAFE
				    doassert = false;
			 #endif
			 {
				JSONNode::auto_lock temp1(test3, 1);  //null, so it should do nothing
				JSONNode::auto_lock temp2(tree, 1);
				assertEquals(testMutex, 1);
			 }
			 #ifndef JSON_SAFE
				    doassert = true;
			 #endif
			 
			 assertEquals(testMutex, 0);
			 
			 #ifdef JSON_MUTEX_MANAGE
				    {
					   JSONNode deleteTest = JSONNode(JSON_NODE);
					   int i = 0;
					   currentMutexTest = &i;
					   deleteTest.set_mutex(&i);
					   std::map<void *, unsigned int>::iterator it = mutex_manager.find((void*)&i);
					   assertEquals(mutex_manager.size(), 2);
					   assertNotEquals(it, mutex_manager.end());
					   assertEquals(it -> first, (void*)&i);
					   assertEquals(it -> second, 1); 
					   
					   deleteTest.set_mutex(&testMutex);
					   currentMutexTest = &testMutex;
				    }
			 #endif
		  }
		  #ifdef JSON_MUTEX_MANAGE
			 std::map<void *, unsigned int>::iterator it = mutex_manager.find((void*)&testMutex);
			 assertEquals(mutex_manager.size(), 0);
			 assertEquals(it, mutex_manager.end());
		  #endif
	   #endif
    }

    #ifdef JSON_MUTEX_CALLBACKS
	   int handler = 0;
	   static void lock_mutex(void * mutex){
		  if (mutex == &handler) return;
		  assertEquals(mutex, &testMutex);
		  if (mutex != &testMutex) return;  //to avoid access violations to tests fail, but don't crash
		  ++(*((int*)mutex));
	   }
	   static void unlock_mutex(void * mutex){
		  if (mutex == &handler) return;
		  assertEquals(mutex, &testMutex);
		  if (mutex != &testMutex) return;  //to avoid access violations to tests fail, but don't crash
		  --(*((int*)mutex));
	   }

	   static void destroy_mutex(void * mutex){}

	   void TestSuite::TestThreading(void){
		  //going to fake two threads os that I don't need pthread to link
		  UnitTest::SetPrefix("Threading");
		  testMutex = 0;
		  #ifdef JSON_LIBRARY
			 //create the JSONNode
			 JSONNODE * test = json_new(JSON_NODE);
			 #ifdef JSON_MUTEX_MANAGE
				json_register_mutex_callbacks(lock_mutex, unlock_mutex, destroy_mutex, &handler);
			 #else
				json_register_mutex_callbacks(lock_mutex, unlock_mutex, &handler);
			 #endif
			 json_set_mutex(test, &testMutex);	 
		  
			 json_lock(test, 1);
			 assertEquals(testMutex, 1);
			 json_lock(test, 1);
			 assertEquals(testMutex, 1);
			 json_lock(test, 2);
			 assertEquals(testMutex, 2);
			 json_unlock(test, 1);
			 assertEquals(testMutex, 2);  //because this thread locked it twice
			 json_unlock(test, 1);
			 assertEquals(testMutex, 1);
			 json_unlock(test, 2);
			 assertEquals(testMutex, 0);
		  
			 json_delete(test);
		  #else
			 //create the JSONNode
			 JSONNode test;
			 #ifdef JSON_MUTEX_MANAGE
				libJSON::register_mutex_callbacks(lock_mutex, unlock_mutex, destroy_mutex, &handler);
			 #else
				libJSON::register_mutex_callbacks(lock_mutex, unlock_mutex, &handler);
			 #endif
		  
			 test.set_mutex(&testMutex);
			 
			 test.lock(1);
			 assertEquals(testMutex, 1);
			 test.lock(1);
			 assertEquals(testMutex, 1);
			 test.lock(2);
			 assertEquals(testMutex, 2);
			 test.unlock(1);
			 assertEquals(testMutex, 2); //because this thread locked it twice
			 test.unlock(1);
			 assertEquals(testMutex, 1);
			 test.unlock(2);
			 assertEquals(testMutex, 0);
		  
		  #endif
	   #endif
    }
#endif
