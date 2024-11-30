// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/ConditionVariable.h>
#include <Bedrock/Test.h>
#include <Bedrock/Thread.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


ConditionVariable::ConditionVariable()
{
	InitializeConditionVariable((PCONDITION_VARIABLE)&mOSCondVar);
}


ConditionVariable::~ConditionVariable()
{
}


void ConditionVariable::NotifyOne()
{
	WakeConditionVariable((PCONDITION_VARIABLE)&mOSCondVar);
}


void ConditionVariable::NotifyAll()
{
	WakeAllConditionVariable((PCONDITION_VARIABLE)&mOSCondVar);
}


ConditionVariable::WaitResult ConditionVariable::Wait(MutexLockGuard& ioLock, NanoSeconds inTimeout)
{
	int timeout_ms = (inTimeout == cInfiniteTimeout) ? INFINITE : (int)gToMilliSeconds(inTimeout);

#ifdef ASSERTS_ENABLED
	// Update the locking thread ID stored inside the mutex, waiting is going to unlock it.
	Mutex* mutex = const_cast<Mutex*>(ioLock.GetMutex());
	uint32 locking_thread_id = mutex->mLockingThreadID;
	mutex->mLockingThreadID = Mutex::cInvalidThreadID;
#endif

	BOOL ret = SleepConditionVariableSRW(
		(PCONDITION_VARIABLE)&mOSCondVar, 
		(PSRWLOCK)&ioLock.GetMutex()->mOSMutex, 
		timeout_ms, 
		0);

#ifdef ASSERTS_ENABLED
	// Put the locking thread ID back.
	gAssert(mutex->mLockingThreadID == Mutex::cInvalidThreadID);
	mutex->mLockingThreadID = locking_thread_id;
#endif

	if (ret != 0)
		return WaitResult::Success;

	gAssert(GetLastError() == ERROR_TIMEOUT);
	return WaitResult::Timeout;
}


REGISTER_TEST("ConditionVariable")
{
	ConditionVariable cond;
	Mutex             mutex;
	int               shared_value = 0;

	LockGuard lock = mutex;

	Thread thread;
	thread.Create({}, [&cond, &mutex, &shared_value](Thread&) 
	{
		LockGuard lock = mutex;

		while (shared_value < 10)
		{
			// Wait until the value is even, then increment it.
			while ((shared_value & 1) == 0)
				cond.Wait(lock);

			shared_value++;
			cond.NotifyOne(); // Notify the other thread. 
		}
	});

	while (shared_value < 10)
	{
		// Wait until the value is odd, then increment it.
		while ((shared_value & 1) == 1)
			cond.Wait(lock);

		shared_value++;
		cond.NotifyOne(); // Notify the other thread. 
	}

	thread.Join();
	TEST_TRUE(shared_value == 11);
};
