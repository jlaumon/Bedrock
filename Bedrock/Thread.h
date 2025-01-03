// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Function.h>
#include <Bedrock/Atomic.h>
#include <Bedrock/String.h>


using OSThread = void*;

struct Thread;

enum class EThreadPriority : int8
{
	Idle,
	Lowest,
	BelowNormal,
	Normal,
	AboveNormal,
	Highest
};


struct ThreadConfig
{
	String          mName        = "";      // The thread name.
	int             mStackSize   = 128_KiB; // The stack size of the thread.
	int             mTempMemSize = 128_KiB; // Initialize a temp memory of that size of the thread. Can be 0.
	EThreadPriority mPriority    = EThreadPriority::Normal; // The priority of the thread.
};


struct Thread : NoCopy
{
	// Default
	Thread() = default;
	~Thread();

	void Create(const ThreadConfig& inConfig, Function<void(Thread&)>&& ioEntryPoint);
	void Join();
	bool IsJoinable() const			{ return mOSThread != nullptr; }
		 
	void RequestStop()				{ mStopRequested.Store(true); }
	bool IsStopRequested() const	{ return mStopRequested.Load(); }

	const ThreadConfig&	GetConfig() const { return mConfig; }

private:
	void Cleanup();

	Function<void(Thread&)> mEntryPoint;
	ThreadConfig            mConfig        = {};
	OSThread                mOSThread      = {};
	uint32                  mOSThreadID    = 0;
	AtomicBool              mStopRequested = false;

	friend struct ThreadInternal;
};


// Yield the processor to other threads that are ready to run.
void gThreadYield();

// Number of threads that can run concurrently.
// Equivalent to the number of CPU cores (incuding hyperthreading logical cores).
int gThreadHardwareConcurrency();