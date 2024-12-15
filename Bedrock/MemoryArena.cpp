// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/MemoryArena.h>
#include <Bedrock/Test.h>

void MemArena::AddPendingFree(FreeBlock inFreeBlock)
{
	for (int i = 0; i < mNumPendingFrees; i++)
	{
		if (inFreeBlock.mEnd < mPendingFrees[i].Begin())
		{
			// Inserting before.
			if (mNumPendingFrees == cMaxPendingFrees) [[unlikely]]
				gCrash("MemArena: too many out of order frees");

			// Move all the blocks towards the back to make room.
			gMemMove(&mPendingFrees[i + 1], &mPendingFrees[i], sizeof(FreeBlock) * (mNumPendingFrees - i));

			// Place the new block.
			mPendingFrees[i] = inFreeBlock;
			mNumPendingFrees++;

			return;
		}

		if (inFreeBlock.mEnd == mPendingFrees[i].Begin())
		{
			// Inserting just before, merge instead.
			mPendingFrees[i].mSize += inFreeBlock.mSize;

			return;
		}

		if (inFreeBlock.Begin() == mPendingFrees[i].mEnd)
		{
			// Inserting just after, merge instead.
			mPendingFrees[i].mEnd = inFreeBlock.mEnd;
			mPendingFrees[i].mSize += inFreeBlock.mSize;

			// Check if the next block can be merged as well now.
			if ((i + 1) < mNumPendingFrees)
			{
				if (mPendingFrees[i].mEnd == mPendingFrees[i + 1].Begin())
				{
					// Merge the next block.
					mPendingFrees[i].mEnd = mPendingFrees[i + 1].mEnd;
					mPendingFrees[i].mSize += mPendingFrees[i + 1].mSize;

					// Move all the following blocks towards the front to fill the gap.
					gMemMove(&mPendingFrees[i + 1], &mPendingFrees[i + 2], sizeof(FreeBlock) * (mNumPendingFrees - 2 - i));
					mNumPendingFrees--;
				}
			}

			return;
		}
	}

	// Otherwise add it at the back of the list.
	if (mNumPendingFrees == cMaxPendingFrees)
		gCrash("MemArena: too many out of order frees");

	mPendingFrees[mNumPendingFrees] = inFreeBlock;
	mNumPendingFrees++;
}


void MemArena::TryRemovePendingFrees()
{
	if (mNumPendingFrees == 0)
		return;

	// Pending blocks are sorted and coalesced, so we only need to check the last one.
	if (mPendingFrees[mNumPendingFrees - 1].mEnd == mCurrent)
	{
		// Free it.
		mCurrent -= mPendingFrees[mNumPendingFrees - 1].mSize;
		mNumPendingFrees--;
	}
}


VMemArena::~VMemArena()
{
	FreeReserved();
}

void VMemArena::CommitMore(uint8* inNewEnd)
{
	gAssert(inNewEnd > mEnd);

	int64    commit_size   = gMax(mCommitIncreaseSize, (inNewEnd - mEnd));
	MemBlock committed_mem = gVMemCommit({ mEnd, commit_size });

	mEnd = committed_mem.mPtr + committed_mem.mSize;
}

void VMemArena::FreeReserved()
{
	if (mBegin != nullptr)
		gVMemFree({ mBegin, mEndReserved - mBegin });
}



REGISTER_TEST("MemArena")
{
	alignas(MemArena::cAlignment) uint8 buffer[MemArena::cAlignment * 5];
	MemArena arena({ buffer, sizeof(buffer) });

	MemBlock b1 = arena.Alloc(1);
	TEST_TRUE(b1 != nullptr);
	MemBlock b2 = arena.Alloc(1);
	TEST_TRUE(b2 != nullptr);
	MemBlock b3 = arena.Alloc(1);
	TEST_TRUE(b3 != nullptr);
	MemBlock b4 = arena.Alloc(1);
	TEST_TRUE(b4 != nullptr);
	MemBlock b5 = arena.Alloc(1);
	TEST_TRUE(b5 != nullptr);

	MemBlock b6 = arena.Alloc(1);
	TEST_TRUE(b6 == nullptr);

	arena.Free(b4);
	TEST_TRUE(arena.GetAllocatedSize() == arena.GetMemBlock().mSize);
	TEST_TRUE(arena.GetNumPendingFree() == 1);
	arena.Free(b2);
	TEST_TRUE(arena.GetAllocatedSize() == arena.GetMemBlock().mSize);
	TEST_TRUE(arena.GetNumPendingFree() == 2);

	arena.Free(b3);
	TEST_TRUE(arena.GetAllocatedSize() == arena.GetMemBlock().mSize);
	TEST_TRUE(arena.GetNumPendingFree() == 1); // All free blocks get merged.

	arena.Free(b1);
	TEST_TRUE(arena.GetAllocatedSize() == arena.GetMemBlock().mSize);
	TEST_TRUE(arena.GetNumPendingFree() == 1);
	arena.Free(b5);
	TEST_TRUE(arena.GetAllocatedSize() == 0);
	TEST_TRUE(arena.GetNumPendingFree() == 0);
};
