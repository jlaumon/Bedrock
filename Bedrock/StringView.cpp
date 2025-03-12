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
	StringView test = "testtest";

	TEST_TRUE(test.Size() == 8);

	TEST_TRUE(test.Find('e') == 1);
	TEST_TRUE(test.Find('z') == -1);
	TEST_TRUE(test.Find('t', 0) == 0);
	TEST_TRUE(test.Find('t', 1) == 3);

	TEST_TRUE(test.Find("test") == 0);
	TEST_TRUE(test.Find("test", 1) == 4);
	TEST_TRUE(test.Find("st") == 2);
	TEST_TRUE(test.Find("") == -1);
	TEST_TRUE(test.Find("ests") == -1);
	TEST_TRUE(test.Contains("st"));
	TEST_FALSE(test.Contains("ests"));

	TEST_TRUE(test.FindFirstOf("t") == 0);
	TEST_TRUE(test.FindFirstOf("es") == 1);
	TEST_TRUE(test.FindFirstOf("zxcv") == -1);

	TEST_TRUE(test.FindLastOf("t") == 7);
	TEST_TRUE(test.FindLastOf("es") == 6);
	TEST_TRUE(test.FindLastOf("zxcv") == -1);

	TEST_TRUE(test.FindFirstNotOf("t") == 1);
	TEST_TRUE(test.FindFirstNotOf("es") == 0);
	TEST_TRUE(test.FindFirstNotOf("zxcv") == 0);
	TEST_TRUE(test.FindFirstNotOf("tes") == -1);

	TEST_TRUE(test.FindLastNotOf("t") == 6);
	TEST_TRUE(test.FindLastNotOf("es") == 7);
	TEST_TRUE(test.FindLastNotOf("zxcv") == 7);
	TEST_TRUE(test.FindLastNotOf("tes") == -1);

	TEST_TRUE(test.SubStr(1) == "esttest");
	TEST_TRUE(test.SubStr(2, 1) == "s");
	TEST_TRUE(test.SubStr(6, 5) == "st");
	TEST_TRUE(test.SubStr(6, -1) == "st"); // Negative count behaves like cMaxInt
	TEST_TRUE(test.SubStr(6, -5) == "st");


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
	TEST_TRUE(test == "testte");

	test.RemovePrefix(2);
	TEST_TRUE(test == "stte");

	test.RemovePrefix(4);
	TEST_TRUE(test == empty);

	{
		StringView cmp = "abcd";
		TEST_TRUE(cmp < "bacd");
		TEST_TRUE(cmp < "abcde");
		TEST_FALSE(cmp < "abc");
		TEST_FALSE(cmp < "abad");
		TEST_FALSE(cmp < "");
		TEST_FALSE(StringView("") < "");
	}
};

