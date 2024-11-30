// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Mutex.h>
#include <Bedrock/Time.h>


using OSCondVar = void*;



struct ConditionVariable : NoCopy
{
	static constexpr NanoSeconds cInfiniteTimeout = (NanoSeconds)-1;

	enum class WaitResult
	{
		Success,
		Timeout
	};


	ConditionVariable();
	~ConditionVariable();

	void NotifyOne();
	void NotifyAll();

	WaitResult Wait(MutexLockGuard& ioLock, NanoSeconds inTimeout = cInfiniteTimeout);

private:
	OSCondVar mOSCondVar = nullptr;
};
