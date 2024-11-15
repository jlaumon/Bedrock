// SPDX-License-Identifier: MPL-2.0
#include<Bedrock/StringView.h>
#include<Bedrock/Hash.h>
#include<Bedrock/Test.h>


uint64 gHash(StringView inValue)
{
	return gHash(inValue.Begin(), inValue.Size());
}


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
	TEST_TRUE(test.Contains("st"));
	TEST_FALSE(test.Contains("ests"));

	TEST_TRUE(test.FindFirstOf("t") == 0);
	TEST_TRUE(test.FindFirstOf("es") == 1);
	TEST_TRUE(test.FindFirstOf("zxcv") == -1);

	TEST_TRUE(test.FindLastOf("t") == 3);
	TEST_TRUE(test.FindLastOf("es") == 2);
	TEST_TRUE(test.FindLastOf("zxcv") == -1);

	TEST_TRUE(test.SubStr(1) == "est");
	TEST_TRUE(test.SubStr(2, 1) == "s");
	TEST_TRUE(test.SubStr(2, 5) == "st");

	StringView empty;

	TEST_TRUE(test.StartsWith("tes"));
	TEST_TRUE(test.StartsWith("test"));
	TEST_TRUE(test.StartsWith(""));
	TEST_TRUE(empty.StartsWith(""));
	TEST_FALSE(test.StartsWith("x"));
	TEST_FALSE(test.StartsWith("test it"));
	TEST_FALSE(empty.StartsWith("test"));

	TEST_TRUE(test.EndsWith("est"));
	TEST_TRUE(test.EndsWith("test"));
	TEST_TRUE(test.EndsWith(""));
	TEST_TRUE(empty.EndsWith(""));
	TEST_FALSE(test.EndsWith("x"));
	TEST_FALSE(test.EndsWith("test it"));
	TEST_FALSE(empty.EndsWith("test"));

	test.RemoveSuffix(2);
	TEST_TRUE(test == "te");
};

