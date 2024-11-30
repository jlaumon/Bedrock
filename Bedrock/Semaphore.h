// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Time.h>


using OSSemaphore = void*;


struct Semaphore : NoCopy
{
	static constexpr NanoSeconds cInfiniteTimeout = (NanoSeconds)-1;

	Semaphore(int inInitialCount, int inMaxCount);
	~Semaphore();

	bool TryAcquire();
	bool Acquire(NanoSeconds inTimeout = cInfiniteTimeout);	// Return true on success, false on timeout.
	void Release(int inCount = 1);

private:

	OSSemaphore mOSSemaphore = {};
};
