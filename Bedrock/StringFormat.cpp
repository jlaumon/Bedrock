// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/StringFormat.h>
#include <Bedrock/Test.h>
#include <Bedrock/String.h>

#include <stdarg.h>

#define STB_SPRINTF_IMPLEMENTATION
#include <Bedrock/thirdparty/stb/stb_sprintf.h>


namespace Details
{
	// The va_list version is kept out of the header to avoid including stdarg.h in the header (or defining it manually) for now.
	void StringFormat(StringFormatCallback inAppendCallback, void* outString, const char* inFormat, va_list inArgs)
	{
		char buffer[STB_SPRINTF_MIN];
		stbsp_vsprintfcb(inAppendCallback, outString, buffer, inFormat, inArgs);
	}
}


void Details::StringFormat(StringFormatCallback inAppendCallback, void* outString, const char* inFormat, ...)
{
	va_list args;
	va_start(args, inFormat);

	StringFormat(inAppendCallback, outString, inFormat, args);

	va_end(args);
}


REGISTER_TEST("StringFormat")
{
	TEST_INIT_TEMP_MEMORY(1_KiB);

	TempString test = "test";

	gFormat(test, "hello %s %d", "world", 1);
	TEST_TRUE(test == "hello world 1");

	gAppendFormat(test, "%s", "!!");
	TEST_TRUE(test == "hello world 1!!");

	String test2 = "test";
	gFormat(test2, "also with %s strings", "non-temp");
	TEST_TRUE(test2 == "also with non-temp strings");
};