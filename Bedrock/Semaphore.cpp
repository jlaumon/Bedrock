// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Semaphore.h>
#include <Bedrock/Test.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


Semaphore::Semaphore(int inInitialCount, int inMaxCount)
{
	mOSSemaphore = CreateSemaphoreA(nullptr, inInitialCount, inMaxCount, nullptr);
}


Semaphore::~Semaphore()
{
	bool success = CloseHandle((HANDLE)mOSSemaphore);
	gAssert(success);
}


bool Semaphore::TryAcquire()
{
	return TryAcquireFor(0_NS);
}


bool Semaphore::TryAcquireFor(NanoSeconds inTimeout)
{
	int timeout_ms = (int)gToMilliSeconds(inTimeout);

	int ret = WaitForSingleObject((HANDLE)mOSSemaphore, timeout_ms);
	return ret == WAIT_OBJECT_0;
}


void Semaphore::Acquire()
{
	int ret = WaitForSingleObject((HANDLE)mOSSemaphore, INFINITE);
	gAssert(ret == WAIT_OBJECT_0);
}


void Semaphore::Release(int inCount)
{
	gAssert(inCount > 0);

	bool success = ReleaseSemaphore((HANDLE)mOSSemaphore, inCount, nullptr);
	gAssert(success); // Going above the max count.
}


REGISTER_TEST("Semaphore")
{
	Semaphore sema(0, 2);

	TEST_FALSE(sema.TryAcquire());

	sema.Release(2);
	sema.Acquire();
	TEST_TRUE(sema.TryAcquireFor(1_MS));
	TEST_FALSE(sema.TryAcquire());
	sema.Release(1);
	TEST_TRUE(sema.TryAcquire());
};
