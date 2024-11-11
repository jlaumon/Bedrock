// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Debug.h>
#include <Bedrock/Core.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


// Check if a debugger is attached.
bool gIsDebuggerAttached()
{
	return IsDebuggerPresent();
}


// Set the name of the current thread.
void gSetCurrentThreadName(const char* inName)
{
	wchar_t wchar_name[256];

	// Convert the name to wide chars.
	int written_wchars = MultiByteToWideChar(CP_UTF8, 0, inName, gStrLen(inName), wchar_name, (int)(gElemCount(wchar_name) - 1));

	if (written_wchars == 0)
		return; // Conversion failed.

	// Make sure the string is null-terminated.
	wchar_name[written_wchars] = 0;

	SetThreadDescription(GetCurrentThread(), wchar_name);
}