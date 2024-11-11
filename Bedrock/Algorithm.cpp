// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Algorithm.h>
#include <Bedrock/StringView.h>
#include <Bedrock/Test.h>

REGISTER_TEST("ReverseIterator")
{
	StringView test = "test";

	ReverseIterator reverse_begin{ test.End() };
	ReverseIterator reverse_end{ test.Begin() };
	TEST_TRUE(reverse_begin != reverse_end);

	ReverseIterator reverse_iter = reverse_begin;
	TEST_TRUE(*reverse_iter == 't'); ++reverse_iter;
	TEST_TRUE(*reverse_iter == 's'); ++reverse_iter;
	TEST_TRUE(*reverse_iter == 'e'); ++reverse_iter;
	TEST_TRUE(*reverse_iter == 't'); ++reverse_iter;
	TEST_TRUE(reverse_iter == reverse_end);
};


REGISTER_TEST("AnyOf")
{
	int values[] = { 1, 2, 3, 4, 5 };

	TEST_TRUE(gAnyOf(values, [](int v) { return v > 3; }));
	TEST_FALSE(gAnyOf(values, [](int v) { return v > 5; }));

	TEST_TRUE(gNoneOf(values, [](int v) { return v > 5; }));
	TEST_FALSE(gNoneOf(values, [](int v) { return v > 3; }));

	TEST_TRUE(gAllOf(values, [](int v) { return v <= 5; }));
	TEST_FALSE(gAllOf(values, [](int v) { return v < 3; }));
};
