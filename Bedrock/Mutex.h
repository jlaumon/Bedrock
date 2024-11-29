// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>


using OSMutex  = void*;
using OSThread = void*;


struct Mutex : NoCopy
{
	Mutex();
	~Mutex();

	void Lock();
	void Unlock();

private:
	OSMutex  mOSMutex       = nullptr;
#ifdef ASSERTS_ENABLED
	OSThread mLockingThread = nullptr;
#endif
};


template <typename taMutex>
struct LockGuard : NoCopy
{
	LockGuard() = default;
	LockGuard(taMutex& ioMutex)
	{
		mMutex = &ioMutex;
		mMutex->Lock();
	}

	~LockGuard()
	{
		if (mMutex)
			mMutex->Unlock();
	}

	LockGuard(LockGuard&& ioOther)
	{
		mMutex         = ioOther.mMutex;
		ioOther.mMutex = nullptr;
	}
	LockGuard& operator=(LockGuard&& ioOther)
	{
		mMutex         = ioOther.mMutex;
		ioOther.mMutex = nullptr;
		return *this;
	}

	void Unlock()
	{
		mMutex->Unlock();
	}

private:
	taMutex* mMutex = nullptr;
};
