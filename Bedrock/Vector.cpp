// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Vector.h>
#include <Bedrock/Test.h>
#include <Bedrock/String.h>


REGISTER_TEST("Vector")
{
	// YOLO compare function.
	auto equal = [](const auto& a, const auto& b) -> bool
	{
		if (a.Size() != b.size())
			return false;

		for (int i = 0; i < a.Size(); i++)
			if (*(a.Begin() + i) != *(b.begin() + i))
				return false;

		return true;
	};

	{
		const Vector<int> test = { 1, 2, 3, 4, 5 };
		TEST_TRUE(test.Size() == 5);
		TEST_TRUE(test.Capacity() >= 5);

		Vector test_copy = test;
		TEST_TRUE(test_copy.Size() == 5);
		TEST_TRUE(test_copy.Capacity() >= 5);
		TEST_TRUE(Span(test_copy) == Span(test));
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
		auto expected = { 0, 1, 2 };
		TEST_TRUE(equal(test_structs, expected));

		struct MovableOnly : TestStruct, NoCopy
		{
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
		auto expected = { 0, 1, 3 };
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
		auto expected = { "0_CopyConstructed", "1_CopyConstructed", "2_CopyConstructed" };
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

	TEST_TRUE(gIsTempMem(test.Begin()));
	TEST_TRUE(test.Size() == 4);
	TEST_TRUE(test.Capacity() >= 4);

	int* test_begin = test.Begin();
	test.Reserve(30);
	TEST_TRUE(test.Begin() == test_begin); // Should have resized the memory block.
	TEST_TRUE(test.Capacity() >= 30);
	TEST_TRUE(test.Size() == 4);

	Vector<int> non_temp = test;
	TEST_TRUE(test.Begin() != non_temp.Begin());
	TEST_TRUE(Span(test) == Span(non_temp));

	non_temp = { 7, 8, 9 };
	test     = non_temp;
	TEST_TRUE(test.Begin() == test_begin);
	TEST_TRUE(test.Begin() != non_temp.Begin());
	TEST_TRUE(Span(test) == Span(non_temp));
};