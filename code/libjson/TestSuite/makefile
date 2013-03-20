single:
	g++ main.cpp TestAssign.cpp TestChildren.cpp \
	TestComments.cpp TestConverters.cpp TestCtors.cpp \
	TestEquality.cpp TestFunctions.cpp TestInequality.cpp \
	TestInspectors.cpp TestIterators.cpp TestMutex.cpp \
	TestNamespace.cpp TestRefCounting.cpp TestSuite.cpp \
	TestWriter.cpp UnitTest.cpp \
	../Source/internalJSONNode.cpp ../Source/JSON_Base64.cpp \
	../Source/JSONChildren.cpp ../Source/JSONDebug.cpp \
	../Source/JSONIterators.cpp ../Source/JSONMemory.cpp \
	../Source/JSONNode_Mutex.cpp ../Source/JSONNode.cpp \
	../Source/JSONWorker.cpp ../Source/JSONWriter.cpp \
	../Source/libJSON.cpp \
     -Wfatal-errors -O3 -ffast-math -fexpensive-optimizations -o testapp
	
debug:
	g++ main.cpp TestAssign.cpp TestChildren.cpp \
	TestComments.cpp TestConverters.cpp TestCtors.cpp \
	TestEquality.cpp TestFunctions.cpp TestInequality.cpp \
	TestInspectors.cpp TestIterators.cpp TestMutex.cpp \
	TestNamespace.cpp TestRefCounting.cpp TestSuite.cpp \
	TestWriter.cpp UnitTest.cpp \
	../Source/internalJSONNode.cpp ../Source/JSON_Base64.cpp \
	../Source/JSONChildren.cpp ../Source/JSONDebug.cpp \
	../Source/JSONIterators.cpp ../Source/JSONMemory.cpp \
	../Source/JSONNode_Mutex.cpp ../Source/JSONNode.cpp \
	../Source/JSONWorker.cpp ../Source/JSONWriter.cpp \
	../Source/libJSON.cpp \
     -Wfatal-errors -DJSON_DEBUG -o testapp

small:
	g++ main.cpp TestAssign.cpp TestChildren.cpp \
	TestComments.cpp TestConverters.cpp TestCtors.cpp \
	TestEquality.cpp TestFunctions.cpp TestInequality.cpp \
	TestInspectors.cpp TestIterators.cpp TestMutex.cpp \
	TestNamespace.cpp TestRefCounting.cpp TestSuite.cpp \
	TestWriter.cpp UnitTest.cpp \
	../Source/internalJSONNode.cpp ../Source/JSON_Base64.cpp \
	../Source/JSONChildren.cpp ../Source/JSONDebug.cpp \
	../Source/JSONIterators.cpp ../Source/JSONMemory.cpp \
	../Source/JSONNode_Mutex.cpp ../Source/JSONNode.cpp \
	../Source/JSONWorker.cpp ../Source/JSONWriter.cpp \
	../Source/libJSON.cpp \
     -Wfatal-errors -Os -ffast-math -DJSON_LESS_MEMORY -o testapp

test:
	g++ All/main.cpp  UnitTest.cpp -O3 -ffast-math -fexpensive-optimizations -o testall
