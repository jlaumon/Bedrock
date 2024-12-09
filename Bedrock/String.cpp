// SPDX-License-Identifier: MPL-2.0
#include<Bedrock/String.h>
#include<Bedrock/Test.h>


REGISTER_TEST("String")
{
	String test = "test";

	TEST_TRUE(test.Size() == 4);
	TEST_TRUE(test.Capacity() >= 5);
	TEST_TRUE(test == "test");
	TEST_TRUE(*test.End() == 0);

	char* test_begin = test.Begin();
	test.Reserve(30);
	TEST_TRUE(test.Begin() != test_begin); // Should have re-allocated.
	TEST_TRUE(test.Capacity() >= 30);
	TEST_TRUE(test.Size() == 4);
	TEST_TRUE(*test.End() == 0);

	test.Resize(2);
	TEST_TRUE(test == "te");
	TEST_TRUE(test.Capacity() >= 30);
	TEST_TRUE(test.Size() == 2);
	TEST_TRUE(*test.End() == 0);

	test = {};
	TEST_TRUE(test.Capacity() >= 30); // Move-assigning an empty string should not free memory.
	TEST_TRUE(test.Size() == 0);

	test = "other_test";
	StringView other_test = test;
	TEST_TRUE(test == other_test);

	StringView test_as_sv = test;
	TEST_TRUE(test == test_as_sv);
	TEST_TRUE(test.Begin() == test_as_sv.Begin());

	test_begin = test.Begin();
	String moved_test = gMove(test);
	TEST_TRUE(moved_test.Begin() == test_begin);
	TEST_TRUE(test.Begin() == StringView().Begin());


	struct TestStringView : StringView
	{
		using StringView::cEmpty; // Making this public for tests 
	};

	// Empty strings all point to the same 1 byte buffer (cEmpty) to avoid allocations.
	String empty;
	TEST_TRUE(empty.AsCStr() == TestStringView::cEmpty);
	empty = "";
	TEST_TRUE(empty.AsCStr() == TestStringView::cEmpty);
	String empty2 = empty;
	TEST_TRUE(empty2.AsCStr() == TestStringView::cEmpty);
	test = "other_test";
	test = empty; // Copy but keep existing alloc.
	TEST_TRUE(test.Empty());
	TEST_TRUE(test.AsCStr() != TestStringView::cEmpty);
	test.Append({});
	TEST_TRUE(test.Empty());
	empty.Append({});
	TEST_TRUE(empty.AsCStr() == TestStringView::cEmpty);
	empty = gMove(test); // Pass alloc to empty.
	TEST_TRUE(empty.AsCStr() != TestStringView::cEmpty);
	TEST_TRUE(test.AsCStr() == TestStringView::cEmpty);

	test.Clear();
	test.Append("test");
	TEST_TRUE(test == "test");
	test.Append("test2");
	TEST_TRUE(test == "testtest2");

	test.RemoveSuffix(4);
	TEST_TRUE(test == "testt");
	TEST_TRUE(*test.End() == 0);
	
	TEST_TRUE(test.Capacity() > test.Size());
	int cap = test.Capacity();
	test.ShrinkToFit();
	TEST_TRUE(test.Capacity() == cap); // ShrinkToFit doesn't do anything with a heap allocator (doesn't support TryReallocate)
};


REGISTER_TEST("TempString")
{
	// Make sure temp memory is initialized or the tests will fail.
	TEST_INIT_TEMP_MEMORY(1_KiB);

	TempString test = "test";

	TEST_TRUE(gIsTempMem(test.Begin()));
	TEST_TRUE(test.Size() == 4);
	TEST_TRUE(test.Capacity() >= 5);
	TEST_TRUE(test == "test");
	TEST_TRUE(*test.End() == 0);

	char* test_begin = test.Begin();
	test.Reserve(30);
	TEST_TRUE(test.Begin() == test_begin); // Should have resized the memory block.
	TEST_TRUE(test.Capacity() >= 30);
	TEST_TRUE(test.Size() == 4);
	TEST_TRUE(*test.End() == 0);

	String non_temp = test;
	TEST_TRUE(test.Begin() != non_temp.Begin());
	TEST_TRUE(test == non_temp);

	non_temp = "other test";
	test     = non_temp;
	TEST_TRUE(test.Begin() == test_begin);
	TEST_TRUE(test.Begin() != non_temp.Begin());
	TEST_TRUE(test == non_temp);

	test.Append("add");
	test.RemoveSuffix(3);
	TEST_TRUE(test.Capacity() > test.Size());
	test.ShrinkToFit();
	TEST_TRUE(test.Capacity() == test.Size() + 1);
};
