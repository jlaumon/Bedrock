// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Function.h>
#include <Bedrock/Test.h>



REGISTER_TEST("Function")
{
	struct CaptureTest
	{
		CaptureTest() = default;
		CaptureTest(const CaptureTest& o)
		{
			mCopy = o.mCopy + 1;
			mMove = o.mMove;
		}
		CaptureTest(CaptureTest&& o)
		{
			mCopy = o.mCopy;
			mMove = o.mMove + 1;
		}

		CaptureTest& operator=(const CaptureTest&) = delete;
		CaptureTest& operator=(CaptureTest&&) = delete;

		int8 mCopy = 0;
		int8 mMove = 0;
	};

	CaptureTest var0, var1, var2;

	Function<void(int)> func = [&captured_ref = var0, captured_copy = var1, captured_move = gMove(var2)](int inExtraMoves) {

		TEST_TRUE(captured_ref.mMove == 0);
		TEST_TRUE(captured_ref.mCopy == 0);

		// One move to Function::mStorage
		TEST_TRUE(captured_copy.mMove == 1 + inExtraMoves);
		TEST_TRUE(captured_copy.mCopy == 1);

		// One move to the captured variable, then one move to Function::mStorage.
		TEST_TRUE(captured_move.mMove == 2 + inExtraMoves);
		TEST_TRUE(captured_move.mCopy == 0);
	};

	func(0);
	
	Function<void(int)> func2 = gMove(func);

	TEST_FALSE(func.IsValid());
	TEST_TRUE(func2.IsValid());

	func2(1);
};

