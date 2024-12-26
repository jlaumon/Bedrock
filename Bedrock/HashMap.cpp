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

	TEST_TRUE(map.Erase("bread"));
	TEST_TRUE(map.Erase("toast"));
	TEST_TRUE(map.Erase("pretzel"));
	TEST_TRUE(map.Erase("brioche"));
	TEST_TRUE(map.Erase("croissant"));
};


REGISTER_TEST("HashSet Reserve")
{
	HashSet<int> set;
	set.Insert(42);

	for (int i = 0; i < 100; i++)
	{
		set.Reserve(i);
		TEST_TRUE(set.Capacity() >= i);
		TEST_TRUE(set.Contains(42));
	}
};


REGISTER_TEST("HashSet")
{
	HashSet<String> set;

	TEST_TRUE(set.Insert("bread").mResult == EInsertResult::Added);
	TEST_TRUE(set.Insert("bread").mResult == EInsertResult::Found);
	TEST_TRUE(set.Insert(StringView("baguette")).mResult == EInsertResult::Added);
	String bagel("bagel");
	TEST_TRUE(set.Insert(bagel).mResult == EInsertResult::Added);
	TEST_TRUE(set.Emplace("bun").mResult == EInsertResult::Added);
	TEST_TRUE(set.Emplace("pretzel").mResult == EInsertResult::Added);
	String brioche("brioche");
	TEST_TRUE(set.Emplace(brioche).mResult == EInsertResult::Added);

	TEST_TRUE(set.Contains("bread"));
	TEST_TRUE(set.Contains(StringView("baguette")));
	TEST_TRUE(set.Contains(bagel));
	TEST_TRUE(set.Contains("bun"));
	TEST_TRUE(set.Contains("pretzel"));
	TEST_TRUE(set.Contains("brioche"));

	TEST_TRUE(set.Insert("ciabatta").mResult == EInsertResult::Added);
	TEST_TRUE(set.Insert("pain").mResult == EInsertResult::Added);
	TEST_TRUE(set.Find("broad") == set.End());

	TEST_TRUE(set.Erase("ciabatta"));
	TEST_TRUE(set.Find("ciabatta") == set.End());
	TEST_TRUE(set.Contains("pain"));
	TEST_FALSE(set.Erase("broad"));

	TEST_TRUE(set.Erase("pretzel"));
	TEST_TRUE(set.Erase("bun"));
	TEST_TRUE(set.Erase("brioche"));
	TEST_TRUE(set.Erase("baguette"));
};


template <class taHashMap>
static void sLargeHashMapTest(taHashMap& map)
{
	constexpr int cSize         = 100000;
	constexpr int cInitialRandSeed = 42;

	// Fill a map with lots of random values.
	int rand_seed = cInitialRandSeed;
	for (int i = 0; i < cSize; i++)
	{
		rand_seed = gRand32(rand_seed);
		map.Insert(i, rand_seed);
	}

	// Check that all the values are found.
	rand_seed = cInitialRandSeed;
	for (int i = 0; i < cSize; i++)
	{
		rand_seed = gRand32(rand_seed);
		auto iter = map.Find(i);
		TEST_TRUE(iter != map.End());
		TEST_TRUE(iter->mKey == i);
		TEST_TRUE(iter->mValue == rand_seed);
	}

	// Make a copy
	decltype(map) map2 = map;

	// Check that all the values are found in copy.
	rand_seed = cInitialRandSeed;
	for (int i = 0; i < cSize; i++)
	{
		rand_seed = gRand32(rand_seed);
		auto iter = map2.Find(i);
		TEST_TRUE(iter != map2.End());
		TEST_TRUE(iter->mKey == i);
		TEST_TRUE(iter->mValue == rand_seed);
	}

	// Remove all the values.
	for (int i = 0; i < cSize; i++)
	{
		TEST_TRUE(map.Erase(i));
	}
}


REGISTER_TEST("Large HashMap")
{
	HashMap<int, int> map;
	sLargeHashMapTest(map);
};


REGISTER_TEST("Large Temp HashMap")
{
	TEST_INIT_TEMP_MEMORY(100_KiB);

	TempHashMap<int, int> map;
	sLargeHashMapTest(map);
};


REGISTER_TEST("Large VMem HashMap")
{
	TEST_INIT_TEMP_MEMORY(100_KiB);

	VMemHashMap<int, int> map;
	sLargeHashMapTest(map);
};



static void sLargeHashSetTest(auto& set)
{
	constexpr int cSize         = 100000;
	constexpr int cInitialRandSeed = 42;

	// Fill a map with lots of random values.
	int rand_value = cInitialRandSeed;
	for (int i = 0; i < cSize; i++)
	{
		rand_value = gRand32(rand_value);
		set.Insert(rand_value);
	}

	// Check that all the values are found.
	rand_value = cInitialRandSeed;
	for (int i = 0; i < cSize; i++)
	{
		rand_value = gRand32(rand_value);
		auto iter = set.Find(rand_value);
		TEST_TRUE(iter != set.End());
		TEST_TRUE(*iter == rand_value);
	}

	// Remove all the values.
	rand_value = cInitialRandSeed;
	for (int i = 0; i < cSize; i++)
	{
		rand_value = gRand32(rand_value);
		TEST_TRUE(set.Erase(rand_value));
	}
}


REGISTER_TEST("Large HashSet")
{
	HashSet<int> set;
	sLargeHashSetTest(set);
};


REGISTER_TEST("Large Temp HashSet")
{
	TEST_INIT_TEMP_MEMORY(100_KiB);

	TempHashSet<int> set;
	sLargeHashSetTest(set);
};


REGISTER_TEST("Large VMem HashSet")
{
	TEST_INIT_TEMP_MEMORY(100_KiB);

	VMemHashSet<int> set;
	sLargeHashSetTest(set);
};
