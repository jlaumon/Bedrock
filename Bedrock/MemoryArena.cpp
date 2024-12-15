// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/MemoryArena.h>
#include <Bedrock/Test.h>



REGISTER_TEST("MemArena")
{
	alignas(MemArena<>::cAlignment) uint8 buffer[MemArena<>::cAlignment * 5];
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
