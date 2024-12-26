// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Memory.h>

namespace Details
{
	struct FreeBlock
	{
		int mEndOffset = 0;
		int mSize      = 0;
		int BeginOffset() const { return mEndOffset - mSize; }
	};

	template <int taSize>
	struct PendingFreeArray
	{
		no_inline    void AddPendingFree(FreeBlock inFreeBlock);
		no_inline    void TryRemovePendingFree(int& ioCurrentOffset);
		force_inline int  GetNumPendingFree() const { return mCount; }

		int       mCount = 0;				
		FreeBlock mBlocks[taSize]; // Sorted in descending order.
	};

	// Specialization that stores nothing.
	template <>	struct PendingFreeArray<0>
	{
		force_inline static void AddPendingFree(FreeBlock inFreeBlock)		{ CRASH; }
		force_inline static void TryRemovePendingFree(int& ioCurrentOffset)	{}
		force_inline static int  GetNumPendingFree()						{ return 0; }
	}; 
}


constexpr int cDefaultMaxPendingFrees = 16;


// Simple linear allocator.
// Allocations must be freed in order in general, but a small number of out of order frees is optionally supported (see taMaxPendingFrees).
// Deals only in bytes. For typed allocators to use with containers, see Allocator.h
template <int taMaxPendingFrees = cDefaultMaxPendingFrees>
struct MemArena : Details::PendingFreeArray<taMaxPendingFrees>
{
	using Base = Details::PendingFreeArray<taMaxPendingFrees>;

	static constexpr int cAlignment       = 16;

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
		Base::operator=((Base&&)ioOther);

		ioOther.mBeginPtr        = nullptr;
		ioOther.mEndOffset       = 0;
		ioOther.mCurrentOffset   = 0;
		ioOther.Base::operator=({});

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
			if (GetNumPendingFree() > 0) [[unlikely]]
				TryRemovePendingFree(mCurrentOffset);
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

	using Base::GetNumPendingFree;

protected:
	uint8* mBeginPtr      = nullptr;
	int    mEndOffset     = 0;
	int    mCurrentOffset = 0;
	
	using Base::AddPendingFree;
	using Base::TryRemovePendingFree;
};


// Version of MemArena that embeds a fixed-size buffer and allocates from it.
template <int taSize, int taMaxPendingFrees = cDefaultMaxPendingFrees>
struct FixedMemArena : MemArena<taMaxPendingFrees>
{
	using Base = MemArena<taMaxPendingFrees>;

	FixedMemArena() : Base({ mBuffer, (int64)taSize }) {}
	~FixedMemArena() { gAssert(Base::GetAllocatedSize() == 0); }

	// Not movable since data is embedded.
	FixedMemArena(FixedMemArena&&)            = delete;
	FixedMemArena& operator=(FixedMemArena&&) = delete;

private:
	alignas(Base::cAlignment) uint8 mBuffer[taSize];
};


// Version of MemArena that allocates virtual memory as backing. Can grow.
template <int taMaxPendingFrees = cDefaultMaxPendingFrees>
struct VMemArena : MemArena<taMaxPendingFrees>
{
	using Base = MemArena<taMaxPendingFrees>;

	static constexpr int64 cDefaultReservedSize = 100_MiB; // By default the arena will reserve that much virtual memory.
	static constexpr int64 cDefaultCommitSize   =  64_KiB; // By default the arena will commit that much virtual memory every time it grows.

	VMemArena() = default;
	~VMemArena() { FreeReserved(); }

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
		Base::operator=(MemBlock{ reserved_mem.mPtr, 0 });
	}

	VMemArena(VMemArena&& ioOther) { operator=((VMemArena&&)ioOther); }
	VMemArena& operator=(VMemArena&& ioOther)
	{
		FreeReserved();

		Base::operator=((Base&&)ioOther);

		mCommitIncreaseSize         = ioOther.mCommitIncreaseSize;
		mEndReservedOffset          = ioOther.mEndReservedOffset;
		ioOther.mEndReservedOffset  = 0;
		ioOther.mCommitIncreaseSize = 0;

		return *this;
	}

	MemBlock Alloc(int inSize)
	{
		// If the arena wasn't initialized yet, do it now (with default values).
		// It's better to do it lazily than reserving virtual memory in every container default constructor.
		if (mBeginPtr == nullptr) [[unlikely]]
			*this = VMemArena(cDefaultReservedSize, cDefaultCommitSize);

		int aligned_size       = (int)gAlignUp(inSize, cAlignment);
		int new_current_offset = mCurrentOffset + aligned_size;

		// Check if we need to commit more memory.
		if (new_current_offset > mEndOffset) [[unlikely]]
			CommitMore(new_current_offset);

		return Base::Alloc(inSize);
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

		return Base::TryRealloc(ioMemory, inNewSize);
	}

	int GetReservedSize() const { return mEndReservedOffset; }

	using Base::IsLastAlloc;
	using Base::cAlignment;

private:
	void CommitMore(int inNewEndOffset);
	void FreeReserved();

	using Base::mBeginPtr;
	using Base::mCurrentOffset;
	using Base::mEndOffset;

	int mEndReservedOffset  = 0;
	int mCommitIncreaseSize = 64_KiB;
};



template <int taSize>
void Details::PendingFreeArray<taSize>::AddPendingFree(FreeBlock inFreeBlock)
{
	for (int i = 0; i < mCount; i++)
	{
		if (inFreeBlock.mEndOffset < mBlocks[i].BeginOffset())
		{
			// Inserting before.
			if (mCount == taSize) [[unlikely]]
				gCrash("MemArena: too many out of order frees");

			// Move all the blocks towards the back to make room.
			gMemMove(&mBlocks[i + 1], &mBlocks[i], sizeof(FreeBlock) * (mCount - i));

			// Place the new block.
			mBlocks[i] = inFreeBlock;
			mCount++;

			return;
		}

		if (inFreeBlock.mEndOffset == mBlocks[i].BeginOffset())
		{
			// Inserting just before, merge instead.
			mBlocks[i].mSize += inFreeBlock.mSize;

			return;
		}

		if (inFreeBlock.BeginOffset() == mBlocks[i].mEndOffset)
		{
			// Inserting just after, merge instead.
			mBlocks[i].mEndOffset = inFreeBlock.mEndOffset;
			mBlocks[i].mSize += inFreeBlock.mSize;

			// Check if the next block can be merged as well now.
			if ((i + 1) < mCount)
			{
				if (mBlocks[i].mEndOffset == mBlocks[i + 1].BeginOffset())
				{
					// Merge the next block.
					mBlocks[i].mEndOffset = mBlocks[i + 1].mEndOffset;
					mBlocks[i].mSize += mBlocks[i + 1].mSize;

					// Move all the following blocks towards the front to fill the gap.
					gMemMove(&mBlocks[i + 1], &mBlocks[i + 2], sizeof(FreeBlock) * (mCount - 2 - i));
					mCount--;
				}
			}

			return;
		}
	}

	// Otherwise add it at the back of the list.
	if (mCount == taSize)
		gCrash("MemArena: too many out of order frees");

	mBlocks[mCount] = inFreeBlock;
	mCount++;

}


template <int taSize>
void Details::PendingFreeArray<taSize>::TryRemovePendingFree(int& ioCurrentOffset)
{
	if (mCount == 0)
		return;

	// Pending blocks are sorted and coalesced, so we only need to check the last one.
	if (mBlocks[mCount - 1].mEndOffset == ioCurrentOffset)
	{
		// Free it.
		ioCurrentOffset -= mBlocks[mCount - 1].mSize;
		mCount--;
	}

}


template <int taMaxPendingFrees>
void VMemArena<taMaxPendingFrees>::CommitMore(int inNewEndOffset)
{
	gAssert(inNewEndOffset > mEndOffset);

	int64    commit_size   = gMax(mCommitIncreaseSize, (inNewEndOffset - mEndOffset));
	MemBlock committed_mem = gVMemCommit({ mBeginPtr + mEndOffset, commit_size });

	mEndOffset = (int)(committed_mem.mPtr + committed_mem.mSize - mBeginPtr);
}

template <int taMaxPendingFrees>
void VMemArena<taMaxPendingFrees>::FreeReserved()
{
	if (mBeginPtr != nullptr)
		gVMemFree({ mBeginPtr, mEndReservedOffset });
}
