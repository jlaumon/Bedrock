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


void gThreadInitTempMemory(MemBlock inMemory)
{
	gAssert(gTempMemBegin == nullptr);							// Already initialized.
	gAssert(((uint64)inMemory.mPtr % cTempMemAlignment) == 0);	// Pointer should be aligned.
	gAssert((inMemory.mSize % cTempMemAlignment) == 0);			// Size should be aligned.

	gTempMemBegin      = inMemory.mPtr;
	gTempMemEnd        = gTempMemBegin + inMemory.mSize;
	gTempMemCurrent    = gTempMemBegin;
}


MemBlock gThreadExitTempMemory()
{
	// Everything should be freed before exiting (or not initialized).
	gAssert(gTempMemCurrent == gTempMemBegin);

	MemBlock memory{ gTempMemBegin, gTempMemEnd - gTempMemBegin };

	gTempMemBegin   = nullptr;
	gTempMemEnd     = nullptr;
	gTempMemCurrent = nullptr;

	return memory;
}



