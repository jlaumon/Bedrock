// SPDX-License-Identifier: MPL-2.0
#pragma once

// Break to the debugger (or crash if no debugger is attached).
#define BREAKPOINT __debugbreak()

// Cause a crash.
#ifdef __clang__
#define CRASH __builtin_trap()
#elif _MSC_VER
extern "C" void __ud2();
#define CRASH __ud2()
#else
#error Unknown compiler
#endif


#ifdef ASSERTS_ENABLED

// Assert macro.
#define gAssert(condition)                                                \
	do                                                                    \
	{                                                                     \
		if (!(condition)) [[unlikely]]                                    \
			if (gReportAssert(#condition, __FILE__, __LINE__)) [[likely]] \
				BREAKPOINT;                                               \
	} while (0)


// Verify macro. Similar to assert, but condition is still executed when asserts are disabled.
#define gVerify(condition) gAssert(condition)


// Internal assert report function. Return true if it should break.
bool gReportAssert(const char* inCondition, const char* inFile, int inLine);

#else

#define gAssert(condition) do { (void)sizeof(!(condition)); } while(0)
#define gVerify(condition) do { (void)(condition); } while(0)

#endif

// Bound checking helper macro.
#define gBoundsCheck(index, size) gAssert((index) >= 0 && (index) < (size))


// Print a message then crash.
[[noreturn]] void gCrash(const char* inMessage);
