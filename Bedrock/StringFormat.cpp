// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/StringFormat.h>
#include <Bedrock/Test.h>
#include <Bedrock/String.h>

#include <stdarg.h>

#define STB_SPRINTF_IMPLEMENTATION
#include <Bedrock/thirdparty/stb/stb_sprintf.h>

#ifdef __SANITIZE_ADDRESS__
#include <cstdio> // For vsnprintf
#endif


void Details::StringFormatV(StringFormatCallback inAppendCallback, void* outString, const char* inFormat, va_list inArgs)
{
	// stb_sprintf triggers needs to turn off ASAN around some of its functions, doesn't do it for MSVC's ASAN.
	// There is a PR pending to fix this but it's been waiting for years... https://github.com/nothings/stb/pull/1350
	// In the meanting, don't use stb_printf when ASAN is enabled, that's the easiest/most reliable workaround.
#ifdef __SANITIZE_ADDRESS__
	// Call vsnprintf a first time to get the string size.
	int size = vsnprintf(nullptr, 0, inFormat, inArgs);
	if (size <= 0)
		return;

	// Allocate a string to store the full string (including null terminator).
	// Note: Do not use a TempString here! inAppendCallback might want to grow an existing TempString.
	String buffer;
	buffer.Reserve(size + 1);

	// Call vsnprintf a second time, with a buffer this time
	size = vsnprintf(buffer.Data(), buffer.Capacity(), inFormat, inArgs);
	if (size <= 0)
		return;

	// Set the correct size and null terminate.
	buffer.Resize(size);

	// Call the user callback.
	inAppendCallback(buffer.Data(), outString, buffer.Size());
#else
	char buffer[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(inAppendCallback, outString, buffer, inFormat, inArgs);
#endif
}


void Details::StringFormat(StringFormatCallback inAppendCallback, void* outString, const char* inFormat, ...)
{
	va_list args;
	va_start(args, inFormat);

	StringFormatV(inAppendCallback, outString, inFormat, args);

	va_end(args);
}


REGISTER_TEST("StringFormat")
{
	TEST_INIT_TEMP_MEMORY(1_KiB);

	TempString test = gTempFormat("hello %s %d", "world", 1);
	TEST_TRUE(test == "hello world 1");

	gAppendFormat(test, "%s", "!!");
	TEST_TRUE(test == "hello world 1!!");

	String test2 = gFormat("also with %s strings", "non-temp");
	TEST_TRUE(test2 == "also with non-temp strings");

	// TempStrings used in gTempFormat means out of order TempMem frees. Make sure this doesn't assert.
	TempString concat = gTempFormat("%s %s %s %s %s %s", 
		TempString("1").AsCStr(),
		TempString("2").AsCStr(),
		TempString("3").AsCStr(),
		TempString("4").AsCStr(),
		TempString("5").AsCStr(),
		TempString("6").AsCStr());
	TEST_TRUE(concat == "1 2 3 4 5 6");
};