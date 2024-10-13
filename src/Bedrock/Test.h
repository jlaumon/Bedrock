// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/StringView.h>

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

void gRegisterTest(StringView inName, TestFunction inFunction); // Register a test. Called automatically by REGISTER_TEST.
bool gRunTests();												// Run all registered tests. Return true on success.
void gFailTest(StringView inMacro, StringView inCode, StringView inFile, int inLine); // Fail current test. Called automatically by the TEST macros.


namespace Details
{
	struct TestDummy {
		StringView mName;
		consteval TestDummy(StringView inName)
		{
			gAssert(!inName.Empty());
			mName = inName;
		}
	};

	struct TestRegisterer { TestRegisterer(StringView inName, TestFunction inFunction) { IF_TESTS_ENABLED(gRegisterTest(inName, inFunction)); } };
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


consteval StringView gConstevalGetFileNamePart(StringView inPath)
{
	int file_start = inPath.FindLastOf("\\/");
	if (file_start == -1)
		return inPath;

	return inPath.SubStr(file_start + 1);
}

#define TEST_TRUE(code) do { if (!(code)) gFailTest("TEST_TRUE", #code, gConstevalGetFileNamePart(__FILE__), __LINE__); } while(0)
#define TEST_FALSE(code) do { if (code) gFailTest("TEST_FALSE", #code, gConstevalGetFileNamePart(__FILE__), __LINE__); } while(0)

