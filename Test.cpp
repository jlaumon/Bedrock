// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Test.h>
#include <Bedrock/Debug.h>
#include <Bedrock/Vector.h>

struct Test
{
	StringView   mName;
	TestFunction mFunction = nullptr;
};

static Vector<Test>& sGetAllTests()
{
	// Return a static variable to avoid issues with globals initialization order.
	static Vector<Test> sAllTests;
	return sAllTests;
}


void gRegisterTest(StringView inName, TestFunction inFunction)
{
	gAssert(!inName.Empty());

	sGetAllTests().PushBack({ inName, inFunction });
}


static thread_local StringView sCurrentTestName;
static thread_local bool       sCurrentTestSuccess;

bool gIsRunningTest()
{
	return !sCurrentTestName.Empty();
}


bool gRunTests()
{
	//gApp.Log("Running all tests.");
	bool all_success = true;

	for (const Test& test : sGetAllTests())
	{
		sCurrentTestName    = test.mName;
		sCurrentTestSuccess = true;
		//gApp.Log(R"(Test "{}" starting.)", test.mName);

		test.mFunction();

		//gApp.Log(R"(Test "{}" finished: {}.)", test.mName, sCurrentTestSuccess ? "Success" : "Failure");
		all_success = all_success && sCurrentTestSuccess;
		sCurrentTestName = "";
	}

	return all_success;
}


void gFailTest(StringView inMacro, StringView inCode, StringView inFile, int inLine)
{
	//gApp.LogError(R"({}({}) failed ({}:{}))", inMacro, inCode, inFile, inLine);

	sCurrentTestSuccess = false;

	// If a debugger is attached, break here.
	if (gIsDebuggerAttached())
		breakpoint;
}


