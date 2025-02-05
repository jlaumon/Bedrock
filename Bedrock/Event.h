// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Time.h>


using OSEvent = void*;


struct Event : NoCopy
{
	enum ResetMode
	{
		AutoReset,
		ManualReset
	};

	Event(ResetMode inResetMode, bool inInitialState = false);
	~Event();

	void Set();
	void Reset();
	
	bool TryWait() const;
	bool TryWaitFor(NanoSeconds inTimeout) const;
	void Wait() const;

private:

	OSEvent mOSEvent = {};
};
