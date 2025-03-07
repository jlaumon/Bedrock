// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Array.h>
#include <Bedrock/Span.h>
#include <Bedrock/Test.h>

REGISTER_TEST("Array")
{
	Array<int, 5> values = { 1, 2, 3, 4, 5 };
	Span test = values;

	// Mostly checking that things compile.
	TEST_TRUE(test == values);

	values.Fill(8);
	for (int value : values)
		TEST_TRUE(value == 8);
};
