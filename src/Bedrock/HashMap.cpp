// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/HashMap.h>
#include <Bedrock/Test.h>
#include <Bedrock/String.h>


REGISTER_TEST("HashMap")
{
	HashMap<String, String> map;
	TEST_TRUE(map.Insert("bread", "butter").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("bread", "jam").mResult == EInsertResult::Found);
	TEST_TRUE(map.Insert("toast", "rubbish").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("baguette", "cheese").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("bagel", "not sure").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("bun", "no").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("pretzel", "fine").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("brioche", "jam").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("croissant", "chocolate").mResult == EInsertResult::Added);

	TEST_TRUE(map.Find("bread")->mValue == "butter");
	TEST_TRUE(map.Find("toast")->mValue == "rubbish");
	TEST_TRUE(map.Find("baguette")->mValue == "cheese");
	TEST_TRUE(map.Find("bagel")->mValue == "not sure");
	TEST_TRUE(map.Find("bun")->mValue == "no");
	TEST_TRUE(map.Find("pretzel")->mValue == "fine");
	TEST_TRUE(map.Find("brioche")->mValue == "jam");
	TEST_TRUE(map.Find("croissant")->mValue == "chocolate");

	TEST_TRUE(map.Insert("ciabatta", "is baguette").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("pain", "perdu").mResult == EInsertResult::Added);
	TEST_TRUE(map.Find("broad") == map.End());

	TEST_TRUE(map.Erase("ciabatta"));
	TEST_TRUE(map.Find("pain")->mValue == "perdu");
	TEST_FALSE(map.Erase("broad"));

};
