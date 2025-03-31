// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TempMemory.h>
#include <Bedrock/Move.h>

// Tests are only enabled in debug. They are compiled but not registered in release (and should be optimized out).
#ifdef ASSERTS_ENABLED
#define TESTS_ENABLED
#endif

#ifdef TESTS_ENABLED
#define IF_TESTS_ENABLED(code) code
#else
#define IF_TESTS_ENABLED(code)
#endif


using TestFunction = void (*)();

enum class TestResult
{
	Success,
	Failure
};


// Register a test. Called automatically by REGISTER_TEST.
void gRegisterTest(const char* inName, TestFunction inFunction); 

// Run all registered tests.
TestResult gRunTests();

// Return true if the current thread is running a test.
bool gIsRunningTest();											 

// Fail current test. Called automatically by the TEST macros.
void gFailTest(const char* inMacro, const char* inCode, const char* inFile, int inLine);

// During tests, memory allocation functions can register their allocations/frees, for leak tracking.
void gRegisterAlloc(MemBlock inMemory);
void gRegisterFree(MemBlock inMemory);

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

// Check that a condition is true. Fail the current test otherwise.
#define TEST_TRUE(code) do { if (!(code)) gFailTest("TEST_TRUE", #code, Details::ConstevalGetFileNamePart(__FILE__), __LINE__); } while(0)

// Check that a condition is false. Fail the current test otherwise.
#define TEST_FALSE(code) do { if (code) gFailTest("TEST_FALSE", #code, Details::ConstevalGetFileNamePart(__FILE__), __LINE__); } while(0)


// Initialize Temporary Memory for the scope of a test.
// eg. TEST_INIT_TEMP_MEMORY(100_KiB);
#define TEST_INIT_TEMP_MEMORY(size_in_bytes) Details::TestScopedTempMemory<size_in_bytes> TOKEN_PASTE(test_temp_mem, __LINE__)


// Get the file name part of a path. eg. file.cpp for path/to/file.cpp
constexpr const char* gGetFileNamePart(const char* inPath)
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


namespace Details
{
	// Same as gGetFileNamePart but consteval
	consteval const char* ConstevalGetFileNamePart(const char* inPath)
	{
		return gGetFileNamePart(inPath);
	}

	// Helper to initialize temporary memory for the scope of a test.
	template <int64 taSize>
	struct TestScopedTempMemory : NoCopy
	{
		[[nodiscard]] TestScopedTempMemory()
		{
			// Save current temp memory setup.
			mSavedTempMemArena   = gMove(gTempMemArena);

			// (Re-)initialize the temp memory with the internal buffer.
			gTempMemArena = {};
			gThreadInitTempMemory({ mBuffer, sizeof(mBuffer) });
		}

		~TestScopedTempMemory()
		{
			// Deinitialize the temp memory. This asserts that everything was freed.
			MemBlock memory = gThreadExitTempMemory();
			gAssert(memory.mPtr == mBuffer);
			gAssert(memory.mSize == sizeof(mBuffer));

			// Restore the saved temp memory setup.
			gTempMemArena   = gMove(mSavedTempMemArena);
		}

		TempMemArena mSavedTempMemArena;

		alignas(TempMemArena::cAlignment) uint8 mBuffer[taSize];
	};
}

