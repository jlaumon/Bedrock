// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

struct MemBlock
{
	uint8* mPtr  = nullptr;
	int64  mSize = 0;

	constexpr bool operator==(NullPtrType) const { return mPtr == nullptr; }
};


// Heap

MemBlock gMemAlloc(int64 inSize);     // Allocate heap memory.
void     gMemFree(MemBlock inMemory); // Free heap memory.


// Virtual Memory

int      gVMemReserveGranularity();      // Return the granularity at which memory can be reserved.
int      gVMemCommitGranularity();       // Return the granularity at which memory can be committed.
MemBlock gVMemReserve(int64 inSize);     // Reserve some memory. inSize will be rounded up to reserve granularity.
void     gVMemFree(MemBlock inMemory);   // Free previously reserved memory.
MemBlock gVMemCommit(MemBlock inMemory); // Commit some reserved memory.
										 // On success, return the committed MemBlock (inMemory rounded up/down to commit granularity).
										 // On failure, return a nullptr MemBlock.



