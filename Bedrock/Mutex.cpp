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
	HANDLE current_thread = GetCurrentThread();
	gAssert(mLockingThread != current_thread); // Recursive locking is not allowed.
#endif

	AcquireSRWLockExclusive((PSRWLOCK)&mOSMutex);

#ifdef ASSERTS_ENABLED
	mLockingThread = current_thread;
#endif
}


void Mutex::Unlock()
{
	gAssert(mLockingThread == GetCurrentThread());

	ReleaseSRWLockExclusive((PSRWLOCK)&mOSMutex);

#ifdef ASSERTS_ENABLED
	mLockingThread = nullptr;
#endif
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
