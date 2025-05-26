// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/FunctionRef.h>
#include <Bedrock/Test.h>



REGISTER_TEST("FunctionRef")
{
	int i = 1;
	int j = 3;

	auto lambda = [i, j](int k) { return i + j + k; };
	FunctionRef<int(int)> func = lambda;
	TEST_TRUE(func(4) == 8);
	TEST_TRUE(func.IsValid());

	func = {};
	TEST_TRUE(!func.IsValid());

	int (*func_ptr)(int) = [](int k) { return k * k; };
	func = func_ptr;
	TEST_TRUE(func(4) == 16);
};

