// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

enum class NanoSeconds : int64;

// Litterals for time durations.
consteval NanoSeconds operator ""_NS(unsigned long long inValue)	{ return (NanoSeconds)inValue; }
consteval NanoSeconds operator ""_US(unsigned long long inValue)	{ return (NanoSeconds)(inValue * 1'000); }
consteval NanoSeconds operator ""_US(long double inValue)			{ return (NanoSeconds)(inValue * 1'000.0); }
consteval NanoSeconds operator ""_MS(unsigned long long inValue)	{ return (NanoSeconds)(inValue * 1'000'000); }
consteval NanoSeconds operator ""_MS(long double inValue)			{ return (NanoSeconds)(inValue * 1'000'000.0); }
consteval NanoSeconds operator ""_S(unsigned long long inValue)		{ return (NanoSeconds)(inValue * 1'000'000'000); }
consteval NanoSeconds operator ""_S(long double inValue)			{ return (NanoSeconds)(inValue * 1'000'000'000.0); }

constexpr double gToMicroSeconds(NanoSeconds inNanoSeconds)	{ return (double)inNanoSeconds / 1'000.0; }
constexpr double gToMilliSeconds(NanoSeconds inNanoSeconds)	{ return (double)inNanoSeconds / 1'000'000.0; }
constexpr double gToSeconds(NanoSeconds inNanoSeconds)		{ return (double)inNanoSeconds / 1'000'000'000.0; }
