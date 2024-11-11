#include <Bedrock/Core.h>
#include <Bedrock/Test.h>

int main(int argc, char* argv[])
{
	// Initialize some temp memory - Trace needs it.
	gThreadInitTempMemory(gMemAlloc(1_MiB));
	defer { gMemFree(gThreadExitTempMemory()); };

	// Run the tests.
	TestResult result = gRunTests();

	return (result == TestResult::Success) ? 0 : 1;
}
