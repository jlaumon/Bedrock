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
};
