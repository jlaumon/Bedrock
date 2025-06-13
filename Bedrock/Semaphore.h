// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Time.h>


using OSSemaphore = void*;


struct Semaphore : NoCopy
{
	Semaphore(int inInitialCount, int inMaxCount);
	~Semaphore();

	bool TryAcquire();
	bool TryAcquireFor(NanoSeconds inTimeout);
	void Acquire();
	void Release(int inCount = 1);

	OSSemaphore GetOSHandle() const { return mOSSemaphore; }

private:

	OSSemaphore mOSSemaphore = {};
};
