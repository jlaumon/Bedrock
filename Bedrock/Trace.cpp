// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Trace.h>
#include <Bedrock/String.h>
#include <Bedrock/StringFormat.h>

#include <Bedrock/thirdparty/stb/stb_sprintf.h>

#include <stdarg.h>
#include <stdio.h>

namespace Details
{
	// The va_list version is kept out of the header to avoid including stdarg.h in the header (or defining it manually) for now.
	void StringFormat(StringFormatCallback inAppendCallback, void* outString, const char* inFormat, va_list inArgs);
}


void Details::Trace(const char* inFormat, ...)
{
	va_list args;
	va_start(args, inFormat);

	TempString string;
	StringFormat(&StringFormatAppendCallback<TempString>, &string, inFormat, args);

	// For now just print to stdout.
	// TODO add timestamps, thread name, log to file, etc.
	puts(string.AsCStr());
}


