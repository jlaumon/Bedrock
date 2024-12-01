// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

#define PUSH_DISABLE_DEPRECATED_WARNING __pragma(warning(push)) __pragma(warning(disable : 4996))
#define POP_WARNING __pragma(warning(pop))

extern "C"
{
	char    _InterlockedExchange8(char volatile*, char);
	char    _InterlockedCompareExchange8(char volatile*, char, char);
	long    _InterlockedExchange(long volatile*, long);
	long    _InterlockedExchangeAdd(long volatile*, long);
	long    _InterlockedCompareExchange(long volatile*, long, long);
	__int8  __iso_volatile_load8(const volatile __int8*);
	__int32 __iso_volatile_load32(const volatile __int32*);
	void    __iso_volatile_store8(volatile __int8*, __int8);
	void    __iso_volatile_store32(volatile __int32*, __int32);
	void    _ReadWriteBarrier();
}

#define COMPILER_BARRIER() PUSH_DISABLE_DEPRECATED_WARNING _ReadWriteBarrier() POP_WARNING

template <typename taType>
struct Atomic;

enum class MemoryOrder
{
	Relaxed,
	//Consume,	// TODO?
	//Acquire,
	//Release,
	//AcqRel,
	SeqCst,
};

template<>
struct Atomic<int32>
{
	Atomic() = default;
	~Atomic() = default;
	Atomic(int32 inValue) : mValue(inValue) {}

	Atomic(const Atomic&) = delete;
	Atomic& operator=(const Atomic&) = delete;

	int32 Load(MemoryOrder inOrder = MemoryOrder::SeqCst) const
	{
		int32 value = __iso_volatile_load32((const int*)&mValue);

		switch (inOrder)
		{
		case MemoryOrder::Relaxed:	break;
		case MemoryOrder::SeqCst:	COMPILER_BARRIER(); break;
		}

		return value;
	}

	void Store(int32 inValue, MemoryOrder inOrder = MemoryOrder::SeqCst)
	{
		switch (inOrder)
		{
		case MemoryOrder::Relaxed:
			__iso_volatile_store32((int*)&mValue, inValue);
			break;
		case MemoryOrder::SeqCst:
			_InterlockedExchange(&mValue, inValue);
			break;
		}
	}

	int32 Exchange(int32 inValue, MemoryOrder inOrder = MemoryOrder::SeqCst)
	{
		return _InterlockedExchange(&mValue, inValue);
	}

	bool CompareExchange(int32& ioExpected, int32 inDesired)
	{
		int32 previous_value = _InterlockedCompareExchange(&mValue, inDesired, ioExpected);

		bool exchanged = (previous_value == ioExpected);

		if (!exchanged)
			ioExpected = previous_value;

		return exchanged;
	}

	int32 Add(int32 inValue, MemoryOrder inOrder = MemoryOrder::SeqCst)
	{
		return _InterlockedExchangeAdd(&mValue, inValue);
	}

	int32 Sub(int32 inValue, MemoryOrder inOrder = MemoryOrder::SeqCst)
	{
		return _InterlockedExchangeAdd(&mValue, -inValue);
	}

private:
	int32 mValue = 0;
};


template<>
struct Atomic<bool>
{
	Atomic() = default;
	~Atomic() = default;
	Atomic(int inValue) : mValue(inValue) {}

	Atomic(const Atomic&) = delete;
	Atomic& operator=(const Atomic&) = delete;

	bool Load(MemoryOrder inOrder = MemoryOrder::SeqCst) const
	{
		bool value = __iso_volatile_load8((const char*)&mValue) != 0;

		switch (inOrder)
		{
		case MemoryOrder::Relaxed:	break;
		case MemoryOrder::SeqCst:	COMPILER_BARRIER(); break;
		}

		return value;
	}

	void Store(bool inValue, MemoryOrder inOrder = MemoryOrder::SeqCst)
	{
		switch (inOrder)
		{
		case MemoryOrder::Relaxed:
			__iso_volatile_store8((char*)&mValue, (char)inValue);
			break;
		case MemoryOrder::SeqCst:
			_InterlockedExchange8((char*)&mValue, (char)inValue);
			break;
		}
	}

	bool Exchange(bool inValue, MemoryOrder inOrder = MemoryOrder::SeqCst)
	{
		return _InterlockedExchange8((char*)&mValue, (char)inValue);
	}

	bool CompareExchange(bool& ioExpected, bool inDesired)
	{
		bool previous_value = (bool)_InterlockedCompareExchange8((char*)&mValue, (char)inDesired, (char)ioExpected);

		bool exchanged = (previous_value == ioExpected);

		if (!exchanged)
			ioExpected = previous_value;

		return exchanged;
	}

private:
	bool mValue = false;
};

using AtomicInt32 = Atomic<int32>;
using AtomicBool = Atomic<bool>;
