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

		mBegin           = ioOther.mBegin;
		mEnd             = ioOther.mEnd;
		mCurrent         = ioOther.mCurrent;
		mNumPendingFrees = ioOther.mNumPendingFrees;
		gMemCopy(mPendingFrees, ioOther.mPendingFrees, mNumPendingFrees * sizeof(FreeBlock));

		ioOther.mBegin           = nullptr;
		ioOther.mEnd             = nullptr;
		ioOther.mCurrent         = nullptr;
		ioOther.mNumPendingFrees = 0;

		return *this;
	}

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

	// Allocate memory.
	MemBlock Alloc(int64 inSize)
	{
		gAssert(mBegin != nullptr); // Need to initialize with a MemBlock first.

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
		gAssert(inMemory.mPtr != nullptr);
		gAssert(inMemory.mSize > 0);
		gAssert(Owns(inMemory.mPtr));

		int64  aligned_size = gAlignUp(inMemory.mSize, cAlignment);
		uint8* end_ptr      = inMemory.mPtr + aligned_size;

		// If it's the last alloc, free immediately.
		if (end_ptr == mCurrent) [[likely]]
		{
			mCurrent -= aligned_size;

			// If there are frees pending because they were made out of order, check if they can be freed now.
			if (mNumPendingFrees > 0) [[unlikely]]
				TryRemovePendingFrees();
		}
		else
		{
			// Otherwise add it to the list of pending frees.
			AddPendingFree({ end_ptr, aligned_size });
		}
	}

	// Try resizing ioMemory. Return true on success. Can fail if ioMemory isn't the last block or not enough free memory for inNewSize.
	bool TryRealloc(MemBlock& ioMemory, int64 inNewSize)
	{
		gAssert(Owns(ioMemory.mPtr));

		if (!IsLastAlloc(ioMemory)) [[unlikely]]
			return false;

		int64 aligned_new_size = gAlignUp(inNewSize, cAlignment);

		if ((ioMemory.mPtr + aligned_new_size) > mEnd) [[unlikely]]
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

	int GetNumPendingFree() const { return mNumPendingFrees; }

protected:
	uint8* mBegin   = nullptr;
	uint8* mEnd     = nullptr;
	uint8* mCurrent = nullptr;
	
	struct FreeBlock
	{
		uint8* mEnd  = nullptr;
		int64  mSize = 0;
		uint8* Begin() const { return mEnd - mSize; }
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

		mCommitIncreaseSize = gAlignUp(inCommitIncreaseSize, gVMemCommitGranularity());

		// Reserve the memory.
		MemBlock reserved_mem = gVMemReserve(inReservedSize);
		mEndReserved = reserved_mem.mPtr + reserved_mem.mSize;

		// Initialize the parent MemArena with a zero-sized block (no memory is committed yet).
		MemArena::operator=(MemBlock{ reserved_mem.mPtr, 0 });
	}

	VMemArena(VMemArena&& ioOther) { operator=((VMemArena&&)ioOther); }
	VMemArena& operator=(VMemArena&& ioOther)
	{
		FreeReserved();

		MemArena::operator=((MemArena&&)ioOther);

		mEndReserved = ioOther.mEndReserved;
		ioOther.mEndReserved = nullptr;

		return *this;
	}

	MemBlock Alloc(int64 inSize)
	{
		gAssert(mBegin != nullptr); // Need to initialize with a MemBlock first.

		int64  aligned_size = gAlignUp(inSize, cAlignment);
		uint8* new_current  = mCurrent + aligned_size;

		// Check if we need to commit more memory.
		if (new_current > mEnd) [[unlikely]]
			CommitMore(new_current);

		return MemArena::Alloc(inSize);
	}

	bool TryRealloc(MemBlock& ioMemory, int64 inNewSize)
	{
		if (!IsLastAlloc(ioMemory)) [[unlikely]]
			return false;

		int64  aligned_new_size = gAlignUp(inNewSize, cAlignment);
		uint8* new_current      = ioMemory.mPtr + aligned_new_size;

		// Check if we need to commit more memory.
		if (new_current > mEnd) [[unlikely]]
			CommitMore(new_current);

		return MemArena::TryRealloc(ioMemory, inNewSize);
	}

private:
	void CommitMore(uint8* inNewEnd);
	void FreeReserved();

	uint8* mEndReserved        = nullptr;
	int64  mCommitIncreaseSize = 64_KiB;
};
