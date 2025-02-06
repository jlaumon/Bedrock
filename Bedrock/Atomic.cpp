// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Atomic.h>
#include <Bedrock/Test.h>
#include <Bedrock/TypeTraits.h>


template <typename taType>
static void sTestAtomic(Atomic<taType>& ioAtomic, MemoryOrder inMemoryOrder, auto inA, auto inB)
{
	ioAtomic.Store(inA, inMemoryOrder);
	TEST_TRUE(ioAtomic.Load(inMemoryOrder) == inA);

	ioAtomic.Store(inB, inMemoryOrder);
	TEST_TRUE(ioAtomic.Load(inMemoryOrder) == inB);

	TEST_TRUE(ioAtomic.Exchange(inA, inMemoryOrder) == inB);

	if constexpr (cIsIntegral<taType> && !cIsSame<taType, bool>)
	{
		gAssert(inA < inB); // Min/max below rely on this

		TEST_TRUE(ioAtomic.Add(inB, inMemoryOrder) == inA);
		TEST_TRUE(ioAtomic.Load(inMemoryOrder) == inA + inB);
		TEST_TRUE(ioAtomic.Sub(inA, inMemoryOrder) == inA + inB);
		TEST_TRUE(ioAtomic.Load(inMemoryOrder) == inB);

		ioAtomic.Store(inA);
		TEST_TRUE(ioAtomic.Max(inB, inMemoryOrder) == inA);
		TEST_TRUE(ioAtomic.Load() == inB);
		TEST_TRUE(ioAtomic.Max(inA, inMemoryOrder) == inB);
		TEST_TRUE(ioAtomic.Load() == inB);

		ioAtomic.Store(inB);
		TEST_TRUE(ioAtomic.Min(inA, inMemoryOrder) == inB);
		TEST_TRUE(ioAtomic.Load() == inA);
		TEST_TRUE(ioAtomic.Min(inB, inMemoryOrder) == inA);
		TEST_TRUE(ioAtomic.Load() == inA);
	}

	// Exchange success
	ioAtomic.Store(inB);
	taType expected = inB;
	TEST_TRUE(ioAtomic.CompareExchange(expected, inA));
	TEST_TRUE(ioAtomic.Load() == inA);
	TEST_TRUE(expected == inB);

	// Exchange failure
	TEST_FALSE(ioAtomic.CompareExchange(expected, inB));
	TEST_TRUE(ioAtomic.Load() == inA);
	TEST_TRUE(expected == inA);
}


REGISTER_TEST("AtomicInt32")
{
	AtomicInt32 atomic;

	sTestAtomic(atomic, MemoryOrder::Relaxed, 1, 9999);
	sTestAtomic(atomic, MemoryOrder::SeqCst, 1, 9999);
};


REGISTER_TEST("AtomicInt8")
{
	AtomicInt8 atomic;

	sTestAtomic(atomic, MemoryOrder::Relaxed, 20, 42);
	sTestAtomic(atomic, MemoryOrder::SeqCst, 20, 42);
};


REGISTER_TEST("AtomicInt64")
{
	AtomicInt64 atomic;

	sTestAtomic(atomic, MemoryOrder::Relaxed, 1000, (int64)cMaxInt * 10);
	sTestAtomic(atomic, MemoryOrder::SeqCst, 1000, (int64)cMaxInt * 10);
};


REGISTER_TEST("AtomicBool")
{
	AtomicBool atomic;

	sTestAtomic(atomic, MemoryOrder::Relaxed, false, true);
	sTestAtomic(atomic, MemoryOrder::SeqCst, false, true);
};


REGISTER_TEST("AtomicObject")
{
	struct Test
	{
		int mValue;

		bool operator==(const Test&) const = default;
	};
	Atomic<Test> atomic;

	sTestAtomic(atomic, MemoryOrder::Relaxed, Test{ 100 }, Test{ 5000 });
	sTestAtomic(atomic, MemoryOrder::SeqCst, Test{ 100 }, Test{ 5000 });
};


REGISTER_TEST("AtomicPtr")
{
	int test;
	Atomic<int*> atomic;

	sTestAtomic(atomic, MemoryOrder::Relaxed, &test, &test + 100);
	sTestAtomic(atomic, MemoryOrder::SeqCst, &test, &test + 100);
};


REGISTER_TEST("AtomicEnum")
{
	enum class TestEnum : int { A, B };
	Atomic<TestEnum> atomic;

	sTestAtomic(atomic, MemoryOrder::Relaxed, TestEnum::A, TestEnum::B);
	sTestAtomic(atomic, MemoryOrder::SeqCst, TestEnum::A, TestEnum::B);
};
