// SPDX-License-Identifier: MPL-2.0
#include<Bedrock/Ticks.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


// Note: QPC resolution is 100ns, but if we ever need more precision we could use tenths of nanosecond here instead.
static int64 sGetNanosecondsPerTick()
{
	static const int64 sNanosecondsPerTick = []
	{
		LARGE_INTEGER frequency { .QuadPart = 1 };
		QueryPerformanceFrequency(&frequency);

		return 1'000'000'000 / frequency.QuadPart;
	}();

	return sNanosecondsPerTick;
}


int64 gGetTickCount()
{
	LARGE_INTEGER counter = {};
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}


int64 gTicksToNanoseconds(int64 inTicks)
{
	return inTicks * sGetNanosecondsPerTick();
}


double gTicksToMilliseconds(int64 inTicks)
{
	int64 ns = gTicksToNanoseconds(inTicks);
	return (double)ns / 1'000'000.0;
}


double gTicksToSeconds(int64 inTicks)
{
	int64 ns = gTicksToNanoseconds(inTicks);
	return (double)ns / 1'000'000'000.0;
}


int64 gNanosecondsToTicks(int64 inNanoseconds)
{
	return inNanoseconds / sGetNanosecondsPerTick();
}


int64 gMillisecondsToTicks(double inMilliseconds)
{
	return gNanosecondsToTicks(int64(inMilliseconds * 1'000'000.0));
}


int64 gSecondsToTicks(double inSeconds)
{
	return gNanosecondsToTicks(int64(inSeconds * 1'000'000'000.0));
}
