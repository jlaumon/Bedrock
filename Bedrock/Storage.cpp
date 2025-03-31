// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Storage.h>
#include <Bedrock/Test.h>


REGISTER_TEST("Storage")
{
	struct TestObject
	{
		TestObject(int& inCounter)
			: mCounter(inCounter)
		{
			mCounter++;
		}

		~TestObject()
		{
			mCounter--;
		}

		int& mCounter;
	};

	int counter = 0;

	{
		Storage<TestObject> object;

		TEST_TRUE(object.IsCreated() == false);
		TEST_TRUE(counter == 0);

		object.Create(counter);

		TEST_TRUE(object.IsCreated());
		TEST_TRUE(counter == 1);
		TEST_TRUE(object->mCounter == counter);
		TEST_TRUE((TestObject*)object != nullptr); // Mostly to check it compiles

		object.Destroy();

		TEST_TRUE(counter == 0);

		object.Create(counter);

		TEST_TRUE(counter == 1);
	}

	TEST_TRUE(counter == 0);
};