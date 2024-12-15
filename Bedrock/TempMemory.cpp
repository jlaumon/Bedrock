// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/TempMemory.h>


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



