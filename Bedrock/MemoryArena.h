// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

struct MemBlock
{
	uint8* mPtr  = nullptr;
	int64  mSize = 0;

	constexpr bool operator==(NullPtrType) const { return mPtr == nullptr; }
};


// Simple linear allocator.
// Allocations should be freed in order, but also supports freeing a small number of allocations out of orders (see taMaxOutOfOrderFrees).
template <int taAlignment = 16, int taMaxOutOfOrderFrees = 16>
struct MemArena
{
	static constexpr int cAlignment = taAlignment;

	MemArena() = default;

	// Initialize this arena with a memory block.
	MemArena(MemBlock inMemory)
	{
		gAssert(mBegin == nullptr);							// Already initialized.
		gAssert(((uint64)inMemory.mPtr % cAlignment) == 0);	// Pointer should be aligned.
		gAssert((inMemory.mSize % cAlignment) == 0);		// Size should be aligned.

		mBegin      = inMemory.mPtr;
		mEnd        = mBegin + inMemory.mSize;
		mCurrent    = mBegin;
	}

	// Get this arena's entire memory block.
	MemBlock GetMemBlock() const
	{
		return { mBegin, mEnd - mBegin };
	}

	// Free all the allocations.
	void Reset()
	{
		mCurrent = mBegin;

		for (int i = 0; i < mNumOutOfOrderFrees; ++i)
			mOutOfOrderFrees[i] = {};
		mNumOutOfOrderFrees = 0;
	}

	// Allocate memory.
	MemBlock Alloc(int64 inSize)
	{
		gAssert(mBegin != nullptr); // Call Init first.

		int64  aligned_size = gAlignUp(inSize, cAlignment);
		uint8* current      = mCurrent;

		if (current + aligned_size > mEnd)
			return {}; // Allocation failed.

		mCurrent += aligned_size;

		return { current, inSize };
	}

	// Free memory. inMemory should be the last allocation, or the arena should support enough out-of-order frees.
	void Free(MemBlock inMemory)
	{
		if (inMemory.mPtr == nullptr)
			return;

		gAssert(inMemory.mSize > 0);
		gAssert(Owns(inMemory.mPtr));

		int64  aligned_size = gAlignUp(inMemory.mSize, cAlignment);
		uint8* end_ptr      = inMemory.mPtr + aligned_size;

		// If it's the last alloc, free immediately.
		if (end_ptr == mCurrent) [[likely]]
		{
			mCurrent -= aligned_size;

			// If there are out of order frees pending, check if they can be freed now.
			for (int i = 0; i < mNumOutOfOrderFrees;)
			{
				if (mOutOfOrderFrees[i].mEnd == mCurrent)
				{
					// Free it.
					mCurrent -= mOutOfOrderFrees[i].mSize;

					// Remove it from the out-of-order free list.
					// Note: A ring buffer might be more efficient since these free blocks are likely in-order,
					// but out-of-order frees should be rare enough that it should not matter.
					gMemMove(&mOutOfOrderFrees[i], &mOutOfOrderFrees[i + 1], sizeof(FreeBlocks) * (mNumOutOfOrderFrees - 1 - i));
					mNumOutOfOrderFrees--;
				}
				else
				{
					i++;
				}
			}
		}
		else
		{
			// Otherwise add it to the list of out-of-order frees.
			gAssert(mNumOutOfOrderFrees < taMaxOutOfOrderFrees);
			mOutOfOrderFrees[mNumOutOfOrderFrees] = { end_ptr, aligned_size };
			mNumOutOfOrderFrees++;
		}
	}

	// Try resizing ioMemory. Return true on success. Can fail if ioMemory isn't the last block or not enough free memory for inNewSize.
	bool TryRealloc(MemBlock& ioMemory, int64 inNewSize)
	{
		gAssert(Owns(ioMemory.mPtr));

		if (!IsLastAlloc(ioMemory))
			return false;

		int64 aligned_new_size = gAlignUp(inNewSize, cAlignment);

		if ((ioMemory.mPtr + aligned_new_size) > mEnd)
			return false; // Wouldn't fit.

		mCurrent = ioMemory.mPtr + aligned_new_size;
		ioMemory.mSize = inNewSize;

		return true;
	}

	// Return true if inMemoryPtr is inside this arena.
	bool Owns(const void* inMemoryPtr) const
	{
		return ((const uint8*)inMemoryPtr >= mBegin && (const uint8*)inMemoryPtr < mEnd);
	}

	// Return true if inMemory is the last allocation made in this arena.
	bool IsLastAlloc(MemBlock inMemory) const
	{
		return (inMemory.mPtr + gAlignUp(inMemory.mSize, cAlignment)) == mCurrent;
	}

	// Return the amount of memory currently allocated.
	int64 GetAllocatedSize() const
	{
		return mCurrent - mBegin;
	}

private:
	uint8* mBegin   = nullptr;
	uint8* mEnd     = nullptr;
	uint8* mCurrent = nullptr;

	struct FreeBlocks
	{
		uint8* mEnd  = nullptr;
		int64  mSize = 0;
	};
	FreeBlocks mOutOfOrderFrees[taMaxOutOfOrderFrees];
	int        mNumOutOfOrderFrees = 0;
};

