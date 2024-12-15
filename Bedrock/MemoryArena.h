// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Memory.h>


// Simple linear allocator.
// Allocations must be freed in order in general, but a small number of out of order frees is supported (see cMaxPendingFrees).
// Deals only in bytes. For typed allocators to use with containers, see Allocator.h
struct MemArena
{
	static constexpr int cAlignment       = 16;
	static constexpr int cMaxPendingFrees = 16;

	// Default
	MemArena()  = default;
	~MemArena() { gAssert(GetAllocatedSize() == 0); }

	// Not copyable
	MemArena(const MemArena&)            = delete;
	MemArena& operator=(const MemArena&) = delete;

	// Move
	MemArena(MemArena&& ioOther) { operator=((MemArena&&)ioOther); }
	MemArena& operator=(MemArena&& ioOther)
	{
		gAssert(GetAllocatedSize() == 0);

		mBeginPtr        = ioOther.mBeginPtr;
		mEndOffset       = ioOther.mEndOffset;
		mCurrentOffset   = ioOther.mCurrentOffset;
		mNumPendingFrees = ioOther.mNumPendingFrees;
		gMemCopy(mPendingFrees, ioOther.mPendingFrees, mNumPendingFrees * sizeof(FreeBlock));

		ioOther.mBeginPtr        = nullptr;
		ioOther.mEndOffset       = 0;
		ioOther.mCurrentOffset   = 0;
		ioOther.mNumPendingFrees = 0;

		return *this;
	}

	// Initialize this arena with a memory block.
	MemArena(MemBlock inMemory)
	{
		gAssert(mBeginPtr == nullptr);						// Already initialized.
		gAssert(((uint64)inMemory.mPtr % cAlignment) == 0);	// Pointer should be aligned.
		gAssert((inMemory.mSize % cAlignment) == 0);		// Size should be aligned.

		mBeginPtr      = inMemory.mPtr;
		mEndOffset     = (int)inMemory.mSize;
		mCurrentOffset = 0;
	}

	// Get this arena's entire memory block.
	MemBlock GetMemBlock() const
	{
		return { mBeginPtr, mEndOffset };
	}

	// Allocate memory.
	MemBlock Alloc(int inSize)
	{
		gAssert(mBeginPtr != nullptr); // Need to initialize with a MemBlock first.

		int aligned_size   = (int)gAlignUp(inSize, cAlignment);
		int current_offset = mCurrentOffset;

		if (current_offset + aligned_size > mEndOffset)
			return {}; // Allocation failed.

		mCurrentOffset += aligned_size;

		return { mBeginPtr + current_offset, inSize };
	}

	// Free memory. inMemory should be the last allocation, or the arena should support enough out-of-order frees.
	void Free(MemBlock inMemory)
	{
		gAssert(inMemory.mPtr != nullptr);
		gAssert(inMemory.mSize > 0);
		gAssert(Owns(inMemory.mPtr));

		int aligned_size = (int)gAlignUp(inMemory.mSize, cAlignment);
		int end_offset   = (int)(inMemory.mPtr + aligned_size - mBeginPtr);

		// If it's the last alloc, free immediately.
		if (end_offset == mCurrentOffset) [[likely]]
		{
			mCurrentOffset -= aligned_size;

			// If there are frees pending because they were made out of order, check if they can be freed now.
			if (mNumPendingFrees > 0) [[unlikely]]
				TryRemovePendingFrees();
		}
		else
		{
			// Otherwise add it to the list of pending frees.
			AddPendingFree({ end_offset, aligned_size });
		}
	}

	// Try resizing ioMemory. Return true on success. Can fail if ioMemory isn't the last block or not enough free memory for inNewSize.
	bool TryRealloc(MemBlock& ioMemory, int inNewSize)
	{
		gAssert(Owns(ioMemory.mPtr));

		if (!IsLastAlloc(ioMemory)) [[unlikely]]
			return false;

		int aligned_new_size   = (int)gAlignUp(inNewSize, cAlignment);
		int new_current_offset = (int)(ioMemory.mPtr + aligned_new_size - mBeginPtr);

		if (new_current_offset > mEndOffset) [[unlikely]]
			return false; // Wouldn't fit.

		mCurrentOffset = new_current_offset;
		ioMemory.mSize = inNewSize;

		return true;
	}

	// Return true if inMemoryPtr is inside this arena.
	bool Owns(const void* inMemoryPtr) const
	{
		return ((const uint8*)inMemoryPtr >= mBeginPtr && (const uint8*)inMemoryPtr < (mBeginPtr + mEndOffset));
	}

	// Return true if inMemory is the last allocation made in this arena.
	bool IsLastAlloc(MemBlock inMemory) const
	{
		return (inMemory.mPtr + gAlignUp(inMemory.mSize, cAlignment)) == (mBeginPtr + mCurrentOffset);
	}

	// Return the amount of memory currently allocated.
	int GetAllocatedSize() const
	{
		return mCurrentOffset;
	}

	int GetNumPendingFree() const { return mNumPendingFrees; }

protected:
	uint8* mBeginPtr      = nullptr;
	int    mEndOffset     = 0;
	int    mCurrentOffset = 0;
	
	struct FreeBlock
	{
		int mEndOffset = 0;
		int mSize      = 0;
		int BeginOffset() const { return mEndOffset - mSize; }
	};

	void AddPendingFree(FreeBlock inFreeBlock);
	void TryRemovePendingFrees();

	int       mNumPendingFrees = 0;				// Sorted in descending order.
	FreeBlock mPendingFrees[cMaxPendingFrees];
};


// Version of MemArena that embeds a fixed-size buffer and allocates from it.
template <int taSize>
struct FixedMemArena : MemArena
{
	FixedMemArena() : MemArena({ mBuffer, (int64)taSize }) {}
	~FixedMemArena() { gAssert(GetAllocatedSize() == 0); }

	// Not movable since data is embedded.
	FixedMemArena(FixedMemArena&&)            = delete;
	FixedMemArena& operator=(FixedMemArena&&) = delete;

private:
	alignas(cAlignment) uint8 mBuffer[taSize];
};


// Version of MemArena that allocates virtual memory as backing. Can grow.
struct VMemArena : MemArena
{
	static constexpr int64 cDefaultReservedSize = 100_MiB; // By default the arena will reserve that much virtual memory.
	static constexpr int64 cDefaultCommitSize   =  64_KiB; // By default the arena will commit that much virtual memory every time it grows.

	VMemArena() = default;
	~VMemArena();

	// Initialize this arena with reserved memory (but no committed memory yet). 
	VMemArena(int64 inReservedSize, int64 inCommitIncreaseSize)
	{
		// Replace parameters by defaults if necessary.
		if (inReservedSize <= 0)
			inReservedSize = cDefaultReservedSize;
		if (inCommitIncreaseSize <= 0)
			inCommitIncreaseSize = cDefaultCommitSize;

		mCommitIncreaseSize = (int)gAlignUp(inCommitIncreaseSize, gVMemCommitGranularity());

		// Reserve the memory.
		MemBlock reserved_mem = gVMemReserve(inReservedSize);
		mEndReservedOffset    = (int)reserved_mem.mSize;

		// Initialize the parent MemArena with a zero-sized block (no memory is committed yet).
		MemArena::operator=(MemBlock{ reserved_mem.mPtr, 0 });
	}

	VMemArena(VMemArena&& ioOther) { operator=((VMemArena&&)ioOther); }
	VMemArena& operator=(VMemArena&& ioOther)
	{
		FreeReserved();

		MemArena::operator=((MemArena&&)ioOther);

		mEndReservedOffset = ioOther.mEndReservedOffset;
		ioOther.mEndReservedOffset = 0;

		return *this;
	}

	MemBlock Alloc(int inSize)
	{
		gAssert(mBeginPtr != nullptr); // Need to initialize with a MemBlock first.

		int aligned_size       = (int)gAlignUp(inSize, cAlignment);
		int new_current_offset = mCurrentOffset + aligned_size;

		// Check if we need to commit more memory.
		if (new_current_offset > mEndOffset) [[unlikely]]
			CommitMore(new_current_offset);

		return MemArena::Alloc(inSize);
	}

	bool TryRealloc(MemBlock& ioMemory, int inNewSize)
	{
		if (!IsLastAlloc(ioMemory)) [[unlikely]]
			return false;

		int aligned_new_size   = (int)gAlignUp(inNewSize, cAlignment);
		int new_current_offset = (int)(ioMemory.mPtr + aligned_new_size - mBeginPtr);

		// Check if we need to commit more memory.
		if (new_current_offset > mEndOffset) [[unlikely]]
			CommitMore(new_current_offset);

		return MemArena::TryRealloc(ioMemory, inNewSize);
	}

private:
	void CommitMore(int inNewEndOffset);
	void FreeReserved();

	int mEndReservedOffset  = 0;
	int mCommitIncreaseSize = 64_KiB;
};
