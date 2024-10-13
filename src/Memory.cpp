// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Memory.h>

#include <stdlib.h>


MemBlock gMemAlloc(int64 inSize)
{
	return { (uint8*)malloc(inSize), inSize };
}


void gMemFree(MemBlock inMemory)
{
	if (inMemory.mPtr == nullptr)
		return;

	gAssert(inMemory.mSize > 0);

	free(inMemory.mPtr);
}


void gThreadInitTempMemory(int64 inTotalSize)
{
	gAssert(gTempMemBegin == nullptr);					// Already initialized.
	gAssert((inTotalSize % cTempMemAlignment) == 0);	// Total size should be aligned.

	gTempMemBegin      = gMemAlloc(inTotalSize).mPtr;
	gTempMemEnd        = gTempMemBegin + inTotalSize;
	gTempMemCurrent    = gTempMemBegin;
}


void gThreadExitTempMemory()
{
	// Everything should be freed before exiting (or not initialized).
	gAssert(gTempMemCurrent == gTempMemBegin);

	gMemFree({ gTempMemBegin, gTempMemEnd - gTempMemBegin });

	gTempMemBegin   = nullptr;
	gTempMemEnd     = nullptr;
	gTempMemCurrent = nullptr;
}



