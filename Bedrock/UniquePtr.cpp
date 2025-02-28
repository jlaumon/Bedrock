// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/UniquePtr.h>
#include <Bedrock/String.h>
#include <Bedrock/Test.h>


REGISTER_TEST("UniquePtr")
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
		UniquePtr ptr(new TestObject(counter));
		TEST_TRUE(counter == 1);
	}
	TEST_TRUE(counter == 0);

	{
		UniquePtr<TestObject> ptr;
		
		TEST_TRUE(ptr == nullptr);
		TEST_TRUE(!ptr);
		TEST_TRUE(counter == 0);

		ptr = UniquePtr(new TestObject(counter));
		TEST_TRUE(counter == 1);

		UniquePtr<TestObject> ptr2 = gMove(ptr);
		TEST_TRUE(counter == 1);
		TEST_TRUE(ptr == nullptr);

		ptr2 = nullptr;
		TEST_TRUE(counter == 0);
	}
};