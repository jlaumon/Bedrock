// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/String.h>


// Format a String and return it.
// Note: The fake call to printf is there to catch format errors. Unlike attribute format, this also works with MSVC.
#define gFormat(format, ...) \
	((void)sizeof(printf(format, __VA_ARGS__)), Details::StringFormatRet<String>(format, __VA_ARGS__))


// Format a string into @a output.
// Note: The fake call to printf is there to catch format errors. Unlike attribute format, this also works with MSVC.
#define gTempFormat(format, ...) \
	((void)sizeof(printf(format, __VA_ARGS__)), Details::StringFormatRet<TempString>(format, __VA_ARGS__))


// Format a string and append to @a output.
// Output can be any String-like class, it only needs an Append(const char*, int) method.
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

	// Template function helper to format a string and return it.
	template <class taString, typename... taArgs>
	taString StringFormatRet(const char* inFormat, taArgs&&... ioArgs)
	{
		taString out_string;
		StringFormat(out_string, inFormat, gForward<taArgs>(ioArgs)...);
		return out_string;
	}
}



// Printf forward declaration for the format validation hack above.
extern "C" int __cdecl printf(const char* inFormat, ...);




