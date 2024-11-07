// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/HashMap.h>
#include <Bedrock/Test.h>
#include <Bedrock/String.h>
#include <Bedrock/Random.h>


REGISTER_TEST("HashMap")
{
	HashMap<String, String> map;
	TEST_TRUE(map.Insert("bread", "butter").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("bread", "jam").mResult == EInsertResult::Found);
	map["toast"] = "rubbish";
	String cheese("cheese");
	TEST_TRUE(map.Insert(StringView("baguette"), cheese).mResult == EInsertResult::Added);
	String bagel("bagel");
	TEST_TRUE(map.Insert(bagel, "not sure").mResult == EInsertResult::Added);
	TEST_TRUE(map.Emplace("bun", "no").mResult == EInsertResult::Added);
	TEST_TRUE(map.Emplace("pretzel", "fine").mResult == EInsertResult::Added);
	String brioche("brioche");
	TEST_TRUE(map.Emplace(brioche, "jam").mResult == EInsertResult::Added);
	TEST_TRUE(map.InsertOrAssign(brioche, "peanut butter").mResult == EInsertResult::Replaced);
	TEST_TRUE(map.InsertOrAssign("croissant", "chocolate").mResult == EInsertResult::Added);

	TEST_TRUE(map.Find("bread")->mValue == "butter");
	TEST_TRUE(map.Find("toast")->mValue == "rubbish");
	TEST_TRUE(map.Find(StringView("baguette"))->mValue == "cheese");
	TEST_TRUE(map.Find(bagel)->mValue == "not sure");
	TEST_TRUE(map["bun"] == "no");
	map["bun"] = "burger";
	TEST_TRUE(map.Find("bun")->mValue == "burger");
	TEST_TRUE(map.Find("pretzel")->mValue == "fine");
	TEST_TRUE(map.Find("brioche")->mValue == "peanut butter");
	TEST_TRUE(map.Find("croissant")->mValue == "chocolate");

	TEST_TRUE(map.Insert("ciabatta", "is baguette").mResult == EInsertResult::Added);
	TEST_TRUE(map.Insert("pain", "perdu").mResult == EInsertResult::Added);
	TEST_TRUE(map.Find("broad") == map.End());

	TEST_TRUE(map.Erase("ciabatta"));
	TEST_TRUE(map.Find("ciabatta") == map.End());
	TEST_TRUE(map.Find("pain")->mValue == "perdu");
	TEST_FALSE(map.Erase("broad"));
};


template <template <typename> typename taAllocator>
static void sLargeHashMapTest()
{
	HashMap<int, int, Hash<int>, taAllocator> map;

	constexpr int cMapSize         = 100000;
	constexpr int cInitialRandSeed = 42;

	// Fill a map with lots of random values.
	int rand_seed = cInitialRandSeed;
	for (int i = 0; i < cMapSize; i++)
	{
		rand_seed = gRand32(rand_seed);
		map.Insert(i, rand_seed);
	}

	// Check that all the values are found.
	rand_seed = cInitialRandSeed;
	for (int i = 0; i < cMapSize; i++)
	{
		rand_seed = gRand32(rand_seed);
		auto iter = map.Find(i);
		TEST_TRUE(iter != map.End());
		TEST_TRUE(iter->mKey == i);
		TEST_TRUE(iter->mValue == rand_seed);
	}
}


REGISTER_TEST("Large HashMap")
{
	sLargeHashMapTest<Allocator>();
};


REGISTER_TEST("Large Temp HashMap")
{
	TEST_INIT_TEMP_MEMORY(100_KiB);

	sLargeHashMapTest<TempAllocator>();
};