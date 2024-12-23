// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Thread.h>
#include <Bedrock/Memory.h>
#include <Bedrock/Debug.h>
#include <Bedrock/Test.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

struct ThreadInternal
{
	static DWORD WINAPI sThreadProc(LPVOID inParam)
	{
		Thread& thread = *(Thread*)inParam;

		// Set the thread name.
		if (!thread.mConfig.mName.Empty())
			gSetCurrentThreadName(thread.mConfig.mName.AsCStr());

		// Allocate temp memory.
		if (thread.mConfig.mTempMemSize > 0)
			gThreadInitTempMemory(gMemAlloc(thread.mConfig.mTempMemSize));

		// Call the entry point.
		thread.mEntryPoint(thread);

		// Free temp memory.
		if (thread.mConfig.mTempMemSize > 0)
			gMemFree(gThreadExitTempMemory());

		return 0;
	}
};


Thread::~Thread()
{
	Join();
}


void Thread::Create(const ThreadConfig& inConfig, Function<void(Thread&)>&& ioEntryPoint)
{
	mEntryPoint = gMove(ioEntryPoint);
	mConfig     = inConfig;

	// Create the thread in a suspended state.
	mOSThread = CreateThread(nullptr, inConfig.mStackSize, ThreadInternal::sThreadProc, this, CREATE_SUSPENDED, &mOSThreadID);
	gAssert(mOSThread != nullptr);

	// Set the priority.
	int priority = THREAD_PRIORITY_NORMAL;
	switch (inConfig.mPriority)
	{
	case EThreadPriority::Idle:			priority = THREAD_PRIORITY_IDLE;			break;
	case EThreadPriority::Lowest:		priority = THREAD_PRIORITY_LOWEST;			break;
	case EThreadPriority::BelowNormal:	priority = THREAD_PRIORITY_BELOW_NORMAL;	break;
	case EThreadPriority::Normal:		priority = THREAD_PRIORITY_NORMAL;			break;
	case EThreadPriority::AboveNormal:	priority = THREAD_PRIORITY_ABOVE_NORMAL;	break;
	case EThreadPriority::Highest:		priority = THREAD_PRIORITY_HIGHEST;			break;
	}
	SetThreadPriority(mOSThread, priority);

	// Start the thread.
	ResumeThread(mOSThread);
}


void Thread::Join()
{
	if (mOSThread == nullptr)
		return;

	// Wait for it to stop.
	WaitForSingleObject(mOSThread, INFINITE);
	CloseHandle(mOSThread);

	Cleanup();
}


// Reset everything to default/empty.
void Thread::Cleanup()
{
	mEntryPoint    = {};
	mConfig        = {};
	mOSThread      = {};
	mOSThreadID    = 0;
	mStopRequested.Store(false);
}


// Number of threads that can run concurrently.
// Equivalent to the number of CPU cores (incuding hyperthreading logical cores).
int gThreadHardwareConcurrency()
{
	static const int number_of_cpus = [] {
		SYSTEM_INFO system_info = {};
		GetSystemInfo(&system_info);
		return gMax(1, (int)system_info.dwNumberOfProcessors);
	}();

	return number_of_cpus;
}


// Yield the processor to other threads that are ready to run.
void gYieldThread()
{
	SwitchToThread();
}


REGISTER_TEST("Thread")
{
	Thread thread;

	bool set_by_thread = false;

	thread.Create(
		{
			.mName        = "TestThread",
			.mTempMemSize = 10_KiB,
		},
		[&set_by_thread](Thread& ioSelf) 
		{
			TEST_TRUE(gTempMemArena.GetMemBlock().mSize == 10_KiB);

			set_by_thread = true;

			while (!ioSelf.IsStopRequested())
				gYieldThread();
		});

	TEST_TRUE(thread.IsStopRequested() == false);
	thread.RequestStop();
	TEST_TRUE(thread.IsStopRequested() == true);

	thread.Join();

	TEST_TRUE(set_by_thread);

	TEST_TRUE(gThreadHardwareConcurrency() >= 1);
};