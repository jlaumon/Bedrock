// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Span.h>
#include <Bedrock/Test.h>

REGISTER_TEST("Span")
{
	int values[] = { 1, 2, 3, 4, 5 };
	Span test = values;

	Span first_two = test.First(2);
	TEST_TRUE(first_two.Size() == 2);
	TEST_TRUE(first_two[0] == 1);
	TEST_TRUE(first_two[1] == 2);

	Span last_two = test.Last(2);
	TEST_TRUE(last_two.Size() == 2);
	TEST_TRUE(last_two[0] == 4);
	TEST_TRUE(last_two[1] == 5);

	TEST_TRUE(test.SubSpan(0) == test);
	TEST_TRUE(test.SubSpan(4, 10).Size() == 1);
	TEST_TRUE(test.SubSpan(4, -1).Size() == 1); // Negative count behaves like cMaxInt
	TEST_TRUE(test.SubSpan(4, -5).Size() == 1);

	// Conversions to const, mostly to check that it compiles.
	Span<const int> const_test = values;
	const_test = test;
	TEST_TRUE(test == const_test);
	TEST_TRUE(const_test == values);
};
