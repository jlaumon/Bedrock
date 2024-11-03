// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>


struct MemBlock
{
	uint8* mPtr  = nullptr;
	int64  mSize = 0;

	constexpr bool operator==(NullPtrType) const { return mPtr == nullptr; }
};


// Allocate heap memory.
MemBlock gMemAlloc(int64 inSize);

// Free heap memory.
void gMemFree(MemBlock inMemory);


// Initialize temporary memory for the current thread.
void gThreadInitTempMemory(int64 inTotalSize);

// De-initialize temporary memory for the current thread.
void gThreadExitTempMemory();

// Allocate stack-like temporary memory.
inline MemBlock gTempMemAlloc(int64 inSize);

// Deallocate stack-like temporary memory.
inline void gTempMemFree(MemBlock inMemory);

// Return true if inMemory is the last allocated block (meaning it can be freed or resized).
inline bool gTempMemIsLast(MemBlock inMemory);

// Return true if inMemoryPtr is temp memory.
inline bool gIsTempMem(const void* inMemoryPtr);

// Try resizing ioMemory. Return true on success. Can fail if ioMemory isn't the last block or not enough free memory for inNewSize.
inline bool gTempMemTryRealloc(MemBlock& ioMemory, int64 inNewSize);


constexpr int64            cTempMemAlignment = 16;
inline thread_local uint8* gTempMemBegin     = nullptr;
inline thread_local uint8* gTempMemEnd       = nullptr;
inline thread_local uint8* gTempMemCurrent   = nullptr;


// Allocate stack-like temporary memory.
MemBlock gTempMemAlloc(int64 inSize)
{
	gAssert(gTempMemBegin != nullptr); // Call gThreadInitTempMemory.

	int64  aligned_size = gAlignUp(inSize, cTempMemAlignment);
	uint8* current      = gTempMemCurrent;

	if (current + aligned_size > gTempMemEnd)
		return {}; // Allocation failed.

	gTempMemCurrent += aligned_size;

	return { current, inSize };
}


// Deallocate stack-like temporary memory.
void gTempMemFree(MemBlock inMemory)
{
	if (inMemory.mPtr == nullptr)
		return;

	gAssert(inMemory.mSize > 0);
	gAssert(gIsTempMem(inMemory.mPtr));
	gAssert(gTempMemIsLast(inMemory)); // Blocks have to be freed in reverse order of allocation.

	int64 aligned_size = gAlignUp(inMemory.mSize, cTempMemAlignment);
	gTempMemCurrent -= aligned_size;
}


// Return true if inMemory is the last allocated block (meaning it can be freed or enlarged).
bool gTempMemIsLast(MemBlock inMemory)
{
	return (inMemory.mPtr + gAlignUp(inMemory.mSize, cTempMemAlignment)) == gTempMemCurrent;
}


// Return true if inMemory is temp memory.
bool gIsTempMem(const void* inMemoryPtr)
{
	return ((const uint8*)inMemoryPtr >= gTempMemBegin && (const uint8*)inMemoryPtr < gTempMemEnd);
}


// Try resizing ioMemory. Return true on success. Can fail if ioMemory isn't the last block or not enough free memory for inNewSize.
bool gTempMemTryRealloc(MemBlock& ioMemory, int64 inNewSize)
{
	gAssert(gIsTempMem(ioMemory.mPtr));

	if (!gTempMemIsLast(ioMemory))
		return false;

	int64 aligned_new_size = gAlignUp(inNewSize, cTempMemAlignment);

	if ((ioMemory.mPtr + aligned_new_size) > gTempMemEnd)
		return false; // Wouldn't fit.

	gTempMemCurrent = ioMemory.mPtr + aligned_new_size;
	ioMemory.mSize = inNewSize;

	return true;
}
