// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Memory.h>
#include <Bedrock/Test.h>

#include <stdlib.h>
#include <Windows.h>

MemBlock gMemAlloc(int64 inSize)
{
	MemBlock memory = { (uint8*)malloc(inSize), inSize };

#ifdef TESTS_ENABLED
	if (gIsRunningTest()) 
		gRegisterAlloc(memory);
#endif

	return memory;
}


void gMemFree(MemBlock inMemory)
{
	gAssert(inMemory.mPtr != nullptr);
	gAssert(inMemory.mSize > 0);

#ifdef TESTS_ENABLED
	if (gIsRunningTest()) 
		gRegisterFree(inMemory);
#endif

	free(inMemory.mPtr);
}


int gVMemReserveGranularity()
{
    static const int sReserveGranularity = []()
	{
		SYSTEM_INFO sys_info;
	    GetSystemInfo(&sys_info);

		gAssert(gIsPow2(sys_info.dwAllocationGranularity));
	    return (int)sys_info.dwAllocationGranularity;
    }();

	return sReserveGranularity;
}


int gVMemCommitGranularity()
{
	static const int sCommitGranularity = []()
	{
		SYSTEM_INFO sys_info;
		GetSystemInfo(&sys_info);

		gAssert(gIsPow2(sys_info.dwPageSize));
		return (int)sys_info.dwPageSize;
	}();

	return sCommitGranularity;
}


MemBlock gVMemReserve(int64 inSize)
{
	inSize = gAlignUp(inSize, gVMemReserveGranularity());
	void* ptr = VirtualAlloc(nullptr, inSize, MEM_RESERVE, PAGE_NOACCESS);

	if (ptr == nullptr) [[unlikely]]
	{
		gAssert(false);
		return {};
	}

	return { (uint8*)ptr, inSize };
}


void gVMemFree(MemBlock inBlock)
{
	BOOL success = VirtualFree(inBlock.mPtr, 0, MEM_RELEASE);
	gAssert(success);
}


MemBlock gVMemCommit(MemBlock inBlock)
{
	int64 begin = (int64)inBlock.mPtr;
	int64 end   = begin + inBlock.mSize;

	// Align the memory block boundaries to page sizes (in case they weren't).
	int64 granularity = gVMemCommitGranularity();
	begin             = gAlignDown(begin, granularity);
	end               = gAlignUp(end, granularity);

	inBlock.mPtr  = (uint8*)begin;
	inBlock.mSize = end - begin;

	void* ptr = VirtualAlloc(inBlock.mPtr, inBlock.mSize, MEM_COMMIT, PAGE_READWRITE);

	if (ptr == nullptr) [[unlikely]]
	{
		gAssert(false);
		return {};
	}

	return inBlock;
}