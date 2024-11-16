// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

// Trace/print a formatted string.
// Note: The fake call to printf is there to catch format errors. Unlike attribute format, this also works with MSVC.
#define gTrace(...)                        \
	do                                     \
	{                                      \
		(void)sizeof(printf(__VA_ARGS__)); \
		Details::Trace(__VA_ARGS__);       \
	} while (false)


namespace Details
{
	// Internal function doing the actual tracing.
	void Trace(const char* inFormat, ...);
}

// Printf forward declaration for the format validation hack above.
extern "C" int __cdecl printf(const char* inFormat, ...);




