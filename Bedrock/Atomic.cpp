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

	if constexpr (!cIsSame<taType, bool>)
	{
		TEST_TRUE(ioAtomic.Add(inB, inMemoryOrder) == inA);
		TEST_TRUE(ioAtomic.Load(inMemoryOrder) == inA + inB);
		TEST_TRUE(ioAtomic.Sub(inA, inMemoryOrder) == inA + inB);
		TEST_TRUE(ioAtomic.Load(inMemoryOrder) == inB);
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



REGISTER_TEST("AtomicBool")
{
	AtomicBool atomic;

	sTestAtomic(atomic, MemoryOrder::Relaxed, true, false);
	sTestAtomic(atomic, MemoryOrder::SeqCst, true, false);
};
