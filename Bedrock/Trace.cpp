// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Trace.h>
#include <Bedrock/String.h>
#include <Bedrock/StringFormat.h>

#include <stdarg.h>
#include <stdio.h>

static TraceCallback sTraceCallback = nullptr;

namespace Details
{
	// The va_list version is kept out of the header to avoid including stdarg.h in the header (or defining it manually) for now.
	void StringFormat(StringFormatCallback inAppendCallback, void* outString, const char* inFormat, va_list inArgs);
}


void Details::Trace(const char* inFormat, ...)
{
	va_list args;
	va_start(args, inFormat);

	TempString string = gTempFormatV(inFormat, args);

	va_end(args);

	// If there's a callback, use it.
	if (sTraceCallback)
	{
		sTraceCallback(string);
	}
	else
	{
		// Otherwise just print to stdout.
		puts(string.AsCStr());
	}
}


void gSetTraceCallback(TraceCallback inCallback)
{
	gAssert(sTraceCallback == nullptr || inCallback == nullptr); // A callback is already set?

	sTraceCallback = inCallback;
}
