// SPDX-License-Identifier: MPL-2.0
#include<Bedrock/Ticks.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


int64 sGetTicksPerSecond()
{
	static const int64 sTicksPerSecond = []
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);

		return frequency.QuadPart;
	}();

	return sTicksPerSecond;
}


int64 gGetTickCount()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}


int64 gTicksToNanoseconds(int64 inTicks)
{
	return inTicks * 1'000'000'000 / sGetTicksPerSecond();
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
	return inNanoseconds * sGetTicksPerSecond() / 1'000'000'000;
}


int64 gMillisecondsToTicks(double inMilliseconds)
{
	return gNanosecondsToTicks(int64(inMilliseconds * 1'000'000.0));
}


int64 gSecondsToTicks(double inSeconds)
{
	return gNanosecondsToTicks(int64(inSeconds * 1'000'000'000.0));
}
