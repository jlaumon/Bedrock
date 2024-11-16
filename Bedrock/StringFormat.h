// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Move.h>

// Format a string into @a output.
// Output can be any String-like class, it only needs Clear() and Append(const char*, int) methods.
// Note: The fake call to printf is there to catch format errors. Unlike attribute format, this also works with MSVC.
#define gFormat(output, format, ...)                        \
	do                                                      \
	{                                                       \
		(void)sizeof(printf(format, __VA_ARGS__));          \
		output.Clear();                                     \
		Details::StringFormat(output, format, __VA_ARGS__); \
	} while (false)


// Format a string and append to @a output.
// Output can be any String-like class, it only needs Append(const char*, int) methods.
// Note: The fake call to printf is there to catch format errors. Unlike attribute format, this also works with MSVC.
#define gAppendFormat(output, format, ...)                  \
	do                                                      \
	{                                                       \
		(void)sizeof(printf(format, __VA_ARGS__));          \
		Details::StringFormat(output, format, __VA_ARGS__); \
	} while (false)


namespace Details
{
	// Callback passed to stbsp_vsprintfcb.
	template <class taString>
	char* StringFormatAppendCallback(const char* inBuffer, void* outString, int inBufferLength)
	{
		taString& output = *(taString*)outString;
		output.Append(inBuffer, inBufferLength);

		// Reuse the same buffer.
		return const_cast<char*>(inBuffer);
	}

	using StringFormatCallback = char*(*)(const char* inBuffer, void* outString, int inBufferLength);

	// Internal function doing the actual formatting.
	void StringFormat(StringFormatCallback inAppendCallback, void* outString, const char* inFormat, ...);

	// Template function helper to automatically get the right callback.
	template <class taString, typename... taArgs>
	void StringFormat(taString& outString, const char* inFormat, taArgs&&... ioArgs)
	{
		StringFormat(&StringFormatAppendCallback<taString>, &outString, inFormat, gForward<taArgs>(ioArgs)...);
	}
}



// Printf forward declaration for the format validation hack above.
extern "C" int __cdecl printf(const char* inFormat, ...);




