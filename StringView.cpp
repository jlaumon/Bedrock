// SPDX-License-Identifier: MPL-2.0
#include<Bedrock/StringView.h>
#include<Bedrock/Test.h>


REGISTER_TEST("StringView")
{
	StringView test = "test";

	TEST_TRUE(test.Size() == 4);

	TEST_TRUE(test.Find('e') == 1);
	TEST_TRUE(test.Find('z') == -1);

	TEST_TRUE(test.Find("test") == 0);
	TEST_TRUE(test.Find("st") == 2);
	TEST_TRUE(test.Find("") == -1);
	TEST_TRUE(test.Find("ests") == -1);

	TEST_TRUE(test.FindFirstOf("t") == 0);
	TEST_TRUE(test.FindFirstOf("es") == 1);
	TEST_TRUE(test.FindFirstOf("zxcv") == -1);

	TEST_TRUE(test.FindLastOf("t") == 3);
	TEST_TRUE(test.FindLastOf("es") == 2);
	TEST_TRUE(test.FindLastOf("zxcv") == -1);

	TEST_TRUE(test.SubStr(1) == "est");
	TEST_TRUE(test.SubStr(2, 1) == "s");
	TEST_TRUE(test.SubStr(2, 5) == "st");
};

