// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Trace.h>
#include <Bedrock/String.h>

#define STB_SPRINTF_IMPLEMENTATION
#include <Bedrock/../../thirdparty/stb/stb_sprintf.h>

#include <stdio.h>

void gTraceInternal(const char* inFormat, ...)
{
	va_list args;
	va_start(args, inFormat);

	TempString buffer;
	buffer.Reserve(STB_SPRINTF_MIN * 2);
	
	auto callback = [](const char*, void* ioContext, int inLength)
	{
		TempString& buffer = *(TempString*)ioContext;

		// Update the current string length.
		buffer.Resize(buffer.Size() + inLength);

		// Reserve more space as necessary.
		buffer.Reserve(buffer.Size() + STB_SPRINTF_MIN);

		return buffer.End();
	};

	stbsp_vsprintfcb(callback, &buffer, buffer.Begin(), inFormat, args);

	va_end(args);

	// For now just print to stdout.
	// TODO add timestamps, thread name, log to file, etc.
	puts(buffer.AsCStr());
}
