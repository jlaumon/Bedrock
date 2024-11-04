// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Vector.h>
#include <Bedrock/Test.h>


REGISTER_TEST("Vector")
{
	const Vector<int> test = { 1, 2, 3, 4, 5 };
	TEST_TRUE(test.Size() == 5);
	TEST_TRUE(test.Capacity() >= 5);

	Vector test_copy = test;
	TEST_TRUE(test_copy.Size() == 5);
	TEST_TRUE(test_copy.Capacity() >= 5);
	TEST_TRUE(Span(test_copy) == Span(test));

	struct TestStruct
	{
		explicit TestStruct(int inValue) : mValue(inValue) {}
		int mValue;
	};

	Vector<TestStruct> test_structs;
	test_structs.PushBack(TestStruct{ 0 });
	test_structs.EmplaceBack(1);
	test_structs.PushBack(TestStruct{ 2 });
	TEST_TRUE(test_structs.Size() == 3);
	TEST_TRUE(test_structs.Capacity() >= 3);
	TEST_TRUE(test_structs[0].mValue == 0);
	TEST_TRUE(test_structs[1].mValue == 1);
	TEST_TRUE(test_structs[2].mValue == 2);

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
	TEST_TRUE(movables[0].mValue == 0);
	TEST_TRUE(movables[1].mValue == 1);
	TEST_TRUE(movables[2].mValue == 2);

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
	TEST_TRUE(copyables[0].mValue == 0);
	TEST_TRUE(copyables[1].mValue == 1);
	TEST_TRUE(copyables[2].mValue == 2);
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