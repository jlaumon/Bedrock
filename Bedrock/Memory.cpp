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
	gAssert(gTempMemArena.GetMemBlock() == nullptr); // Already initialized.

	gTempMemArena = { inMemory };
}


MemBlock gThreadExitTempMemory()
{
	// Everything should be freed before exiting (or not initialized).
	gAssert(gTempMemArena.GetAllocatedSize() == 0);

	MemBlock mem_block = gTempMemArena.GetMemBlock();
	gTempMemArena      = {};
	return mem_block;
}



