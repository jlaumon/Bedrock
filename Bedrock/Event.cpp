// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Event.h>
#include <Bedrock/Test.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


Event::Event(ResetMode inResetMode, bool inInitialState)
{
	mOSEvent = CreateEventA(nullptr, inResetMode == ManualReset, inInitialState, nullptr);
	gAssert(mOSEvent != nullptr);
}


Event::~Event()
{
	bool success = CloseHandle((HANDLE)mOSEvent);
	gAssert(success);
}


void Event::Set()
{
	bool success = SetEvent((HANDLE)mOSEvent);
	gAssert(success);
}


void Event::Reset()
{
	bool success = ResetEvent((HANDLE)mOSEvent);
	gAssert(success);
}


bool Event::TryWait() const
{
	return TryWaitFor(0_NS);
}


bool Event::TryWaitFor(NanoSeconds inTimeout) const
{
	int timeout_ms = (int)gToMilliSeconds(inTimeout);

	int ret = WaitForSingleObject((HANDLE)mOSEvent, timeout_ms);
	return ret == WAIT_OBJECT_0;
}


void Event::Wait() const
{
	int ret = WaitForSingleObject((HANDLE)mOSEvent, INFINITE);
	gAssert(ret == WAIT_OBJECT_0);
}


REGISTER_TEST("Event")
{
	{
		Event event(Event::ManualReset, false);

		TEST_FALSE(event.TryWait());

		event.Set();
		TEST_TRUE(event.TryWait());
		
		event.Reset();
		TEST_FALSE(event.TryWait());
	}

	{
		Event event(Event::AutoReset, false);

		TEST_FALSE(event.TryWait());

		event.Set();
		TEST_TRUE(event.TryWait());
		
		TEST_FALSE(event.TryWait());
	}
};
