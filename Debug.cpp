// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Debug.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


// Check if a debugger is attached.
bool gIsDebuggerAttached()
{
	return IsDebuggerPresent();
}

