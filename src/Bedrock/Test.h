// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

// Tests are only enabled in debug. They are compiled but not registered in release (and should be optimized out).
#ifdef ASSERTS_ENABLED
#define TESTS_ENABLED
#endif

#ifdef TESTS_ENABLED
#define IF_TESTS_ENABLED(code) code
#else
#define IF_TESTS_ENABLED(code)
#endif

bool gIsRunningTest();											// Return true if the current thread is running a test.

using TestFunction = void (*)();

void gRegisterTest(const char* inName, TestFunction inFunction); // Register a test. Called automatically by REGISTER_TEST.
bool gRunTests();												// Run all registered tests. Return true on success.
void gFailTest(const char* inMacro, const char* inCode, const char* inFile, int inLine); // Fail current test. Called automatically by the TEST macros.


namespace Details
{
	struct TestDummy {
		const char* mName;
		consteval TestDummy(const char* inName)
		{
			gAssert(inName != nullptr && inName[0] != 0);
			mName = inName;
		}
	};

	struct TestRegisterer { TestRegisterer(const char* inName, TestFunction inFunction) { IF_TESTS_ENABLED(gRegisterTest(inName, inFunction)); } };
	inline TestRegisterer operator*(TestDummy inDummy, TestFunction inFunction) { return { inDummy.mName, inFunction }; }
}

// Register a test.
// eg.
//
// REGISTER_TEST("Examples")
// {
//		TEST_TRUE(gGetTheAnswer() == 42);
//		TEST_FALSE(gIsTooManyCooks());
// };
#define REGISTER_TEST(name) static auto TOKEN_PASTE(test_register, __LINE__) = Details::TestDummy{ name } *[]()

namespace Details
{
	consteval const char* gConstevalGetFileNamePart(const char* inPath)
	{
		int after_last_slash = 0;
		int i = 0;
		while (inPath[i] != 0)
		{
			if (inPath[i] == '/' || inPath[i] == '\\')
				after_last_slash = i + 1;
			i++;
		}

		return inPath + after_last_slash;
	}
}

#define TEST_TRUE(code) do { if (!(code)) gFailTest("TEST_TRUE", #code, Details::gConstevalGetFileNamePart(__FILE__), __LINE__); } while(0)
#define TEST_FALSE(code) do { if (code) gFailTest("TEST_FALSE", #code, Details::gConstevalGetFileNamePart(__FILE__), __LINE__); } while(0)

