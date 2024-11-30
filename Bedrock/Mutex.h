// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

struct ConditionVariable;

using OSMutex  = void*;
using OSThread = void*;


struct Mutex : NoCopy
{
	Mutex();
	~Mutex();

	void Lock();
	void Unlock();

private:
	static constexpr uint32 cInvalidThreadID = 0;

	OSMutex  mOSMutex         = nullptr;
#ifdef ASSERTS_ENABLED
	uint32   mLockingThreadID = cInvalidThreadID;
#endif

	friend struct ConditionVariable;
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

	const Mutex* GetMutex() const { return mMutex; }

private:
	taMutex* mMutex = nullptr;
};

using MutexLockGuard = LockGuard<Mutex>;