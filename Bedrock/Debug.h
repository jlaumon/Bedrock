// SPDX-License-Identifier: MPL-2.0
#pragma once

// Check if a debugger is attached.
bool gIsDebuggerAttached();

// Set the name of the current thread.
void gSetCurrentThreadName(const char* inName);