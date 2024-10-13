// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Memory.h>


// Default allocator.
template <typename taType>
struct Allocator
{
	// Allocate memory.
	static taType*	Allocate(int64 inSize)					{ return (taType*)gMemAlloc(inSize * sizeof(taType)).mPtr; }
	static void		Free(taType* inPtr, int64 inSize)		{ return gMemFree({ (uint8*)inPtr, inSize * (int64)sizeof(taType) }); }

	// Try growing an existing allocation, return false if unsuccessful.
	static bool		TryGrow(taType* inPtr, int64 inCurrentSize, int64 inNewSize)	{ return false; }
};



// Temp memory allocator. Fallsback to the default allocator if temp memory runs out.
template <typename taType>
struct TempAllocator : Allocator<taType>
{
	// Allocate memory.
	static taType*	Allocate(int64 inSize);
	static void     Free(taType* inPtr, int64 inSize);

	// Try growing an existing allocation, return false if unsuccessful.
	static bool		TryGrow(taType* inPtr, int64 inCurrentSize, int64 inNewSize);
};


template <typename taType>
taType* TempAllocator<taType>::Allocate(int64 inSize)
{
	MemBlock mem = gTempMemAlloc(inSize * sizeof(taType));

	if (mem != nullptr)
		return (taType*)mem.mPtr;

	return Allocator<taType>::Allocate(inSize);
}


template <typename taType>
void TempAllocator<taType>::Free(taType* inPtr, int64 inSize)
{
	if (gIsTempMem(inPtr))
		gTempMemFree({ (uint8*)inPtr, inSize * (int64)sizeof(taType) });
	else
		Allocator<taType>::Free(inPtr, inSize);
}


template <typename taType>
bool TempAllocator<taType>::TryGrow(taType* inPtr, int64 inCurrentSize, int64 inNewSize)
{
	if (gIsTempMem(inPtr))
	{
		MemBlock mem = { (uint8*)inPtr, inCurrentSize * (int64)sizeof(taType) };
		return gTempMemTryRealloc(mem, inNewSize * (int64)sizeof(taType));
	}
	
	return Allocator<taType>::TryGrow(inPtr, inCurrentSize, inNewSize);
}


