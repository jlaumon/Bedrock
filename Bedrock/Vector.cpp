// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Vector.h>
#include <Bedrock/Test.h>
#include <Bedrock/String.h>


REGISTER_TEST("Vector")
{
	// YOLO compare function.
	auto equal = [](const auto& a, const auto& b) -> bool
	{
		if (a.Size() != b.Size())
			return false;

		for (int i = 0; i < a.Size(); i++)
			if (*(a.Begin() + i) != *(b.Begin() + i))
				return false;

		return true;
	};

	{
		Vector<int> test = { 1, 2, 3, 4, 5 };
		TEST_TRUE(test.Size() == 5);
		TEST_TRUE(test.Capacity() >= 5);

		Vector test_copy = test;
		TEST_TRUE(test_copy.Size() == 5);
		TEST_TRUE(test_copy.Capacity() >= 5);
		TEST_TRUE(Span(test_copy) == Span(test));

		test_copy = gMove(test); // test_copy should free its previous alloc. Leak detection will tell.
		TEST_TRUE(test.Empty());
	}

	{
		struct TestStruct
		{
			explicit TestStruct(int inValue) : mValue(inValue) {}
			bool operator==(int inValue) const { return inValue == mValue; }
			int mValue;
		};

		Vector<TestStruct> test_structs;
		test_structs.PushBack(TestStruct{ 0 });
		test_structs.EmplaceBack(1);
		test_structs.PushBack(TestStruct{ 2 });
		TEST_TRUE(test_structs.Size() == 3);
		TEST_TRUE(test_structs.Capacity() >= 3);
		Vector<int> expected = { 0, 1, 2 };
		TEST_TRUE(equal(test_structs, expected));

		struct MovableOnly : TestStruct
		{
			MovableOnly(MovableOnly&&)				   = default;
			MovableOnly& operator=(MovableOnly&&)      = default;
			MovableOnly(const MovableOnly&)            = delete;
			MovableOnly& operator=(const MovableOnly&) = delete;
			using TestStruct::TestStruct;
		};

		Vector<MovableOnly> movables;
		movables.PushBack(MovableOnly{ 0 });
		movables.EmplaceBack(1);
		movables.PushBack(MovableOnly{ 2 });
		TEST_TRUE(movables.Size() == 3);
		TEST_TRUE(movables.Capacity() >= 3);
		TEST_TRUE(equal(movables, expected));

		struct CopyableOnly : TestStruct
		{
			using TestStruct::TestStruct;
			CopyableOnly(const CopyableOnly&)            = default;
			CopyableOnly& operator=(const CopyableOnly&) = default;
			CopyableOnly(CopyableOnly&&)                 = delete;
			CopyableOnly& operator=(CopyableOnly&&)      = delete;
		};

		Vector<CopyableOnly> copyables;
		CopyableOnly         c(0);
		copyables.PushBack(c);
		copyables.EmplaceBack(1);
		c.mValue = 2;
		copyables.PushBack(c);
		TEST_TRUE(copyables.Size() == 3);
		TEST_TRUE(copyables.Capacity() >= 3);
		TEST_TRUE(equal(copyables, expected));
	}

	{
		Vector<int> test = { 0, 3 };
		test.Emplace(1, 1);
		Vector<int> expected = { 0, 1, 3 };
		TEST_TRUE(equal(test, expected));

		const int& i2 = 2;
		test.Insert(2, i2);
		expected = { 0, 1, 2, 3 };
		TEST_TRUE(equal(test, expected));

		test.Insert(4, 4);
		expected = { 0, 1, 2, 3, 4 };
		TEST_TRUE(equal(test, expected));

		int arr1[] = { 8, 9 };
		test.Insert(5, arr1);
		expected = { 0, 1, 2, 3, 4, 8, 9 };
		TEST_TRUE(equal(test, expected));

		int arr2[] = { 5, 6, 7 };
		test.Insert(5, arr2);
		expected = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		TEST_TRUE(equal(test, expected));

		test.Erase(5);
		expected = { 0, 1, 2, 3, 4, 6, 7, 8, 9 };
		TEST_TRUE(equal(test, expected));

		test.Erase(0);
		expected = { 1, 2, 3, 4, 6, 7, 8, 9 };
		TEST_TRUE(equal(test, expected));

		TEST_TRUE(test.Capacity() > test.Size());
		int cap = test.Capacity();
		test.ShrinkToFit();
		TEST_TRUE(test.Capacity() == cap); // ShrinkToFit doesn't do anything with a heap allocator (doesn't support TryReallocate)
	}

	{
		Vector<int> test;

		test.Resize(5, 99);

		Vector<int> expected = { 99, 99, 99, 99, 99 };
		TEST_TRUE(equal(test, expected));

		test.Resize(1);
		expected = { 99 };
		TEST_TRUE(equal(test, expected));

		test.Resize(3);
		expected = { 99, 0, 0 };
		TEST_TRUE(equal(test, expected));

		test.Resize(5, EResizeInit::NoZeroInit);
		expected = { 99, 0, 0, 99, 99 };
		TEST_TRUE(equal(test, expected));
	}

	{
		struct CopyMoveTest
		{
			CopyMoveTest(const char* inValue)
			{
				mValue = inValue;
				gAssert(mValue != "");
			}
			CopyMoveTest(const CopyMoveTest& o)
			{
				mValue = o.mValue.SubStr(0, 1);
				mValue += "_CopyConstructed";
			}
			CopyMoveTest(CopyMoveTest&& o)
			{
				mValue = o.mValue.SubStr(0, 1);
				mValue += "_MoveConstructed";
				o.mValue = "Moved";
			}
			CopyMoveTest& operator=(const CopyMoveTest& o)
			{
				mValue = o.mValue.SubStr(0, 1);
				mValue += "_CopyAssigned";
				return *this;
			}
			CopyMoveTest& operator=(CopyMoveTest&& o)
			{
				mValue = o.mValue.SubStr(0, 1);
				mValue += "_MoveAssigned";
				o.mValue = "Moved";
				return *this;
			}
			~CopyMoveTest() = default;

			bool operator==(StringView inValue) const { return inValue == mValue; }

			String mValue;
		};

		Vector<CopyMoveTest> test = { "0", "1", "2" };
		Vector<StringView> expected = { "0_CopyConstructed", "1_CopyConstructed", "2_CopyConstructed" };
		TEST_TRUE(equal(test, expected));

		test.Reserve(10);
		expected = { "0_MoveConstructed", "1_MoveConstructed", "2_MoveConstructed" };
		TEST_TRUE(equal(test, expected));

		test.Emplace(1, "3");
		expected = { "0_MoveConstructed", "3", "1_MoveAssigned", "2_MoveConstructed" };
		TEST_TRUE(equal(test, expected));

		test.Emplace(4, "4");
		expected = { "0_MoveConstructed", "3", "1_MoveAssigned", "2_MoveConstructed", "4" };
		TEST_TRUE(equal(test, expected));

		CopyMoveTest arr[] = { "5", "6", "7" };
		test.Insert(4, arr);
		expected = { "0_MoveConstructed", "3", "1_MoveAssigned", "2_MoveConstructed", "5_CopyAssigned", "6_CopyConstructed", "7_CopyConstructed", "4_MoveConstructed" };
		TEST_TRUE(equal(test, expected));

		test.Erase(1, 3);
		expected = { "0_MoveConstructed", "5_MoveAssigned", "6_MoveAssigned", "7_MoveAssigned", "4_MoveAssigned" };
		TEST_TRUE(equal(test, expected));

		test.Erase(4);
		expected = { "0_MoveConstructed", "5_MoveAssigned", "6_MoveAssigned", "7_MoveAssigned" };
		TEST_TRUE(equal(test, expected));

		test.Erase(0);
		expected = { "5_MoveAssigned", "6_MoveAssigned", "7_MoveAssigned" };
		TEST_TRUE(equal(test, expected));
	}
};


REGISTER_TEST("TempVector")
{
	TEST_INIT_TEMP_MEMORY(1_KiB);

	TempVector<int> test = { 1, 2, 3, 4 };

	TEST_TRUE(gTempMemArena.Owns(test.Begin()));
	TEST_TRUE(test.Size() == 4);
	TEST_TRUE(test.Capacity() >= 4);

	int* test_begin = test.Begin();
	test.Reserve(30);
	TEST_TRUE(test.Begin() == test_begin); // Should have resized the memory block.
	TEST_TRUE(test.Capacity() >= 30);
	TEST_TRUE(test.Size() == 4);

	Vector<int> heap_vec = test;
	TEST_TRUE(test.Begin() != heap_vec.Begin());
	TEST_TRUE(Span(test) == Span(heap_vec));

	heap_vec = { 7, 8, 9 };
	test     = heap_vec;
	TEST_TRUE(test.Begin() == test_begin);
	TEST_TRUE(test.Begin() != heap_vec.Begin());
	TEST_TRUE(Span(test) == Span(heap_vec));

	test.PushBack(1);
	test.PopBack();
	TEST_TRUE(test.Capacity() > test.Size());
	test.ShrinkToFit();
	TEST_TRUE(test.Capacity() == test.Size());
};


REGISTER_TEST("ArenaVector")
{
	FixedMemArena<1_KiB> mem_arena;

	ArenaVector<int> test(mem_arena);
	test = { 1, 2, 3, 4 };

	TEST_TRUE(mem_arena.Owns(test.Begin()));
	TEST_TRUE(test.Size() == 4);
	TEST_TRUE(test.Capacity() >= 4);

	int* test_begin = test.Begin();
	test.Reserve(30);
	TEST_TRUE(test.Begin() == test_begin); // Should have resized the memory block.
	TEST_TRUE(test.Capacity() >= 30);
	TEST_TRUE(test.Size() == 4);

	Vector<int> heap_vec = test;
	TEST_TRUE(test.Begin() != heap_vec.Begin());
	TEST_TRUE(Span(test) == Span(heap_vec));

	heap_vec = { 7, 8, 9 };
	test     = heap_vec;
	TEST_TRUE(test.Begin() == test_begin);
	TEST_TRUE(test.Begin() != heap_vec.Begin());
	TEST_TRUE(Span(test) == heap_vec);

	test.PushBack(1);
	test.PopBack();
	TEST_TRUE(test.Capacity() > test.Size());
	test.ShrinkToFit();
	TEST_TRUE(test.Capacity() == test.Size());
	
	FixedMemArena<128_B> mem_arena2;
	ArenaVector<int>     test2(mem_arena2);

	// Copy to a different arena
	test2 = test;
	TEST_TRUE(test2 == Span(test));
	TEST_TRUE(mem_arena2.Owns(test2.Begin()));
	TEST_TRUE(test2.GetAllocator().GetArena() == &mem_arena2);

	// Move also moves the arena
	test2 = gMove(test);
	TEST_TRUE(Span(test2) == heap_vec);
	TEST_TRUE(test.Empty());
	TEST_TRUE(mem_arena.Owns(test2.Begin()));
	TEST_TRUE(test2.GetAllocator().GetArena() == &mem_arena);
	TEST_TRUE(test.GetAllocator().GetArena() == nullptr);
};


REGISTER_TEST("VMemVector")
{
	VMemVector<int> test;
	test = { 1, 2, 3, 4 };

	TEST_TRUE(test.Size() == 4);
	TEST_TRUE(test.Capacity() >= 4);

	int* test_begin = test.Begin();
	test.Reserve(30);
	TEST_TRUE(test.Begin() == test_begin); // Should have resized the memory block.
	TEST_TRUE(test.Capacity() >= 30);
	TEST_TRUE(test.Size() == 4);

	Vector<int> heap_vec = test;
	TEST_TRUE(test.Begin() != heap_vec.Begin());
	TEST_TRUE(Span(test) == Span(heap_vec));

	heap_vec = { 7, 8, 9 };
	test     = heap_vec;
	TEST_TRUE(test.Begin() == test_begin);
	TEST_TRUE(test.Begin() != heap_vec.Begin());
	TEST_TRUE(Span(test) == heap_vec);

	test.PushBack(1);
	test.PopBack();
	TEST_TRUE(test.Capacity() > test.Size());
	test.ShrinkToFit();
	TEST_TRUE(test.Capacity() == test.Size());

	VMemVector<int> test2(VMemVector<int>::Allocator(8_KiB, 4_KiB));

	test2 = test;
	TEST_TRUE(test2 == Span(test));

	// This should cause a new page to be committed.
	test2.Resize(5_KiB / sizeof(test[0]), EResizeInit::NoZeroInit);
	test2.PushBack(1); // Make sure writing to it doesn't crash.

	test2 = gMove(test);
	TEST_TRUE(Span(test2) == heap_vec);
	TEST_TRUE(test.Empty());
};


REGISTER_TEST("FixedVector")
{
	FixedVector<int, 6> test;
	test = { 1, 2, 3, 4, 5 };

	TEST_TRUE(test.Size() == 5);
	TEST_TRUE(test.Capacity() == 5);
	TEST_TRUE(test.MaxSize() == 6);

	test.PushBack(6); // Should not grow above max size
	TEST_TRUE(test.Capacity() == 6);

	int* test_begin = test.Begin();

	Vector<int> heap_vec = test;
	TEST_TRUE(test.Begin() != heap_vec.Begin());
	TEST_TRUE(Span(test) == Span(heap_vec));

	heap_vec = { 7, 8, 9 };
	test     = heap_vec;
	TEST_TRUE(test.Begin() == test_begin);
	TEST_TRUE(test.Begin() != heap_vec.Begin());
	TEST_TRUE(Span(test) == heap_vec);

	test.PushBack(1);
	test.PopBack();
	TEST_TRUE(test.Capacity() > test.Size());
	test.ShrinkToFit();
	TEST_TRUE(test.Capacity() == test.Size());
	
	FixedVector<int, 8> test2;

	// Copy to a different vector
	test2 = test;
	TEST_TRUE(test2 == Span(test));
	TEST_TRUE(test2.Data() != test.Data());

	// Move also copies since there's no move operator
	test2 = gMove(test);
	TEST_TRUE(Span(test2) == heap_vec);
	TEST_TRUE(!test.Empty());
	TEST_TRUE(test2.Data() != test.Data());
};