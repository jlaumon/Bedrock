// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Mutex.h>
#include <Bedrock/Test.h>
#include <Bedrock/Move.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


Mutex::Mutex()
{
	InitializeSRWLock((PSRWLOCK)&mOSMutex); 
}


Mutex::~Mutex()
{
}


void Mutex::Lock()
{
#ifdef ASSERTS_ENABLED
	uint32 current_thread_id = GetCurrentThreadId();
	gAssert(mLockingThreadID != current_thread_id); // Recursive locking is not allowed.
#endif

	AcquireSRWLockExclusive((PSRWLOCK)&mOSMutex);

#ifdef ASSERTS_ENABLED
	mLockingThreadID = current_thread_id;
#endif
}


void Mutex::Unlock()
{
#ifdef ASSERTS_ENABLED
	gAssert(mLockingThreadID == GetCurrentThreadId());
	mLockingThreadID = cInvalidThreadID;
#endif

	ReleaseSRWLockExclusive((PSRWLOCK)&mOSMutex);
}



REGISTER_TEST("Mutex")
{
	Mutex mutex;

	mutex.Lock();
	mutex.Unlock();

	{
		LockGuard lock(mutex);

		LockGuard<Mutex> other_lock;

		other_lock = gMove(lock);
	}
};
