#ifndef _TEST_SUITE_H_
#define _TEST_SUITE_H_

#include <string>
#include <sstream>
#include <cstring>

class UnitTest {
public:
    static void SelfCheck(void);
    static void PushFailure(const std::string & fail);
    static void PushSuccess(const std::string & pass);
    static void echo_(const std::string & out);
    static std::string ToString(void);
    static std::string ToHTML(void);
    static void SaveTo(const std::string & location);
    static void SetReturnOnFail(bool option);
    static bool GetReturnOnFail(void);
    static void SetEcho(bool option);
    static void SetPrefix(const std::string & prefix);
    static std::string GetPrefix(void);
    static void StartTime(void);
};

#define MakePre()\
    std::string pre = UnitTest::GetPrefix();\
    if (pre.empty()){\
        std::stringstream out;\
        out << __FILE__ << ":" << __LINE__;\
        pre = out.str();\
    }\
    pre += ":  ";

#define FAIL(stri)\
    MakePre()\
    UnitTest::PushFailure(pre + std::string(stri));\
    if (UnitTest::GetReturnOnFail()) return;

#define PASS(stri)\
    MakePre();\
    UnitTest::PushSuccess(pre + std::string(stri));\

#define assertUnitTest()\
    UnitTest::SelfCheck();

#define assertTrue(cond)\
    if (!(cond)){\
	   FAIL(#cond);\
    } else {\
	   PASS(#cond);\
    }

#define assertFalse(cond)\
    if (cond){\
	   FAIL(#cond);\
    } else {\
	   PASS(#cond);\
    }

#define assertEquals(one, two)\
    assertTrue((one) == (two))

#define assertNotEquals(one, two)\
    assertTrue((one) != (two))

#define assertGreaterThan(one, two)\
    assertTrue((one) > (two))

#define assertGreaterThanEqualTo(one, two)\
    assertTrue((one) >= (two))

#define assertLessThan(one, two)\
    assertTrue((one) < (two))

#define assertLessThanEqualTo(one, two)\
    assertTrue((one) <= (two))

#define assertNull(one)\
    assertTrue(one == NULL);

#define assertNotNull(one)\
    assertTrue(one != NULL);

#define assertCStringEquals(one, two)\
    if (strcmp(one, two)){\
	   FAIL(std::string(#one) + "==" + #two);\
    } else {\
	   PASS(std::string(#one) + "==" + #two);\
    }

#define assertCStringNotEquals(one, two)\
    if (!strcmp(one, two)){\
	   FAIL(std::string(#one) + "!=" + #two);\
    } else {\
	   PASS(std::string(#one) + "!=" + #two);\
    }

#define assertCStringEqualsW(one, two)\
    if (wcscmp(one, two)){\
	   FAIL(std::string(#one) + "==" + #two);\
    } else {\
	   PASS(std::string(#one) + "==" + #two);\
    }

#define assertCStringNotEqualsW(one, two)\
    if (!wcscmp(one, two)){\
	   FAIL(std::string(#one) + "!=" + #two);\
    } else {\
	   PASS(std::string(#one) + "!=" + #two);\
    }

#define assertException(code, exc)\
    {\
	   bool failed = false;\
	   try {\
		  code;\
	   } catch (exc){\
		  PASS(std::string(#exc) + " caught");\
		  failed = true;\
	   }\
	   if (!failed){ FAIL(std::string(#exc) + " not caught");}\
    }

#define echo(something)\
    {\
	   std::stringstream somet;\
	   somet << something;\
	   UnitTest::echo_(somet.str());\
    }

#endif
