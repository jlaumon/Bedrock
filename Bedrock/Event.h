// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Time.h>


using OSEvent = void*;


struct Event : NoCopy
{
	enum ResetMode
	{
		AutoReset,	// The event is reset automatically after waiting on it.
		ManualReset // The event stays set until Reset is called.
	};

	Event(ResetMode inResetMode, bool inInitialState = false);
	~Event();

	void Set();	  // Set the event.
	void Reset(); // Reset the event.
	
	bool TryWait() const;						  // Return true if the event is set.
	bool TryWaitFor(NanoSeconds inTimeout) const; // Wait until the event is set (return true), or until timeout (return false).
	void Wait() const;							  // Wait until the event is set.

	OSEvent GetOSHandle() const { return mOSEvent; }
private:

	OSEvent mOSEvent = {};
};
