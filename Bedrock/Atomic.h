// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TypeTraits.h>

#define PUSH_DISABLE_DEPRECATED_WARNING __pragma(warning(push)) __pragma(warning(disable : 4996))
#define POP_WARNING __pragma(warning(pop))


extern "C"
{
	char    _InterlockedExchange8(char volatile*, char);
	char    _InterlockedExchangeAdd8(char volatile*, char);
	char    _InterlockedCompareExchange8(char volatile*, char, char);
	long    _InterlockedExchange(long volatile*, long);
	long    _InterlockedExchangeAdd(long volatile*, long);
	long    _InterlockedCompareExchange(long volatile*, long, long);
	__int64 _InterlockedExchange64(__int64 volatile*, __int64);
	__int64 _InterlockedExchangeAdd64(__int64 volatile*, __int64);
	__int64 _InterlockedCompareExchange64(__int64 volatile*, __int64, __int64);
	__int8  __iso_volatile_load8(const volatile __int8*);
	__int32 __iso_volatile_load32(const volatile __int32*);
	__int64 __iso_volatile_load64(const volatile __int64*);
	void    __iso_volatile_store8(volatile __int8*, __int8);
	void    __iso_volatile_store32(volatile __int32*, __int32);
	void    __iso_volatile_store64(volatile __int64*, __int64);
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


// Atomic template class.
// Supports integral types (including bool), pointers and trivially copyable classes <= 8 bytes. Lock-free only.
template<typename taType>
struct Atomic : NoCopy
{
	static_assert(cIsTriviallyCopyable<taType>);
	static_assert(sizeof(taType) == 8 || sizeof(taType) == 4 || sizeof(taType) == 1); // Note: int16 not supported only because not needed so far.

	using ValueType = taType;
	using StorageType = Conditional<sizeof(taType) == 8, int64, Conditional<sizeof(taType) == 4, int32, char>>; // An integral type that can hold the value.

	static_assert(sizeof(ValueType) == sizeof(StorageType));

	Atomic() = default;
	~Atomic() = default;
	Atomic(ValueType inValue) : mValue(inValue) {}

	ValueType	Load(MemoryOrder inOrder = MemoryOrder::SeqCst) const;
	void		Store(ValueType inValue, MemoryOrder inOrder = MemoryOrder::SeqCst);

	ValueType	Exchange(ValueType inValue, MemoryOrder inOrder = MemoryOrder::SeqCst);
	bool		CompareExchange(ValueType& ioExpected, ValueType inDesired);

	ValueType	Add(ValueType inValue, MemoryOrder inOrder = MemoryOrder::SeqCst) requires cIsIntegral<taType> && (!cIsSame<taType, bool>);
	ValueType	Sub(ValueType inValue, MemoryOrder inOrder = MemoryOrder::SeqCst) requires cIsIntegral<taType> && (!cIsSame<taType, bool>);

private:
	static force_inline ValueType	sAsValue(StorageType inStorage);
	static force_inline StorageType	sAsStorage(ValueType inValue);

	ValueType	mValue; // Note: This could be StorageType, but ValueType looks nicer in the debugger.
};


// Typedefs for convenience.
using AtomicInt8 = Atomic<int8>;
using AtomicInt32 = Atomic<int32>;
using AtomicInt64 = Atomic<int64>;
using AtomicBool = Atomic<bool>;


template <typename taType>
typename Atomic<taType>::ValueType Atomic<taType>::Load(MemoryOrder inOrder) const
{
	StorageType value;
	auto        storage_ptr = (const StorageType*)&mValue;

	if constexpr(sizeof(taType) == 8)
		value = __iso_volatile_load64(storage_ptr);
	else if constexpr(sizeof(taType) == 4)
		value = __iso_volatile_load32((const int*)storage_ptr);
	else
		value = __iso_volatile_load8(storage_ptr);

	switch (inOrder)
	{
	case MemoryOrder::Relaxed:	break;
	case MemoryOrder::SeqCst:	COMPILER_BARRIER(); break;
	}

	return sAsValue(value);
}


template <typename taType>
void Atomic<taType>::Store(ValueType inValue, MemoryOrder inOrder)
{
	StorageType new_value   = sAsStorage(inValue);
	auto        storage_ptr = (StorageType*)&mValue;

	switch (inOrder)
	{
	case MemoryOrder::Relaxed: 
	{
		if constexpr(sizeof(taType) == 8)
			__iso_volatile_store64(storage_ptr, new_value);
		else if constexpr(sizeof(taType) == 4)
			__iso_volatile_store32((int*)storage_ptr, new_value);
		else
			__iso_volatile_store8(storage_ptr, new_value);
		break;
	}
			
	case MemoryOrder::SeqCst:
		if constexpr(sizeof(taType) == 8)
			_InterlockedExchange64(storage_ptr, new_value);
		else if constexpr(sizeof(taType) == 4)
			_InterlockedExchange(storage_ptr, new_value);
		else
			_InterlockedExchange8(storage_ptr, new_value);
		break;
	}
}


template <typename taType>
typename Atomic<taType>::ValueType Atomic<taType>::Exchange(ValueType inValue, MemoryOrder inOrder)
{
	StorageType new_value   = sAsStorage(inValue);
	auto        storage_ptr = (StorageType*)&mValue;
	StorageType previous_value;

	if constexpr(sizeof(taType) == 8)
		previous_value = _InterlockedExchange64(storage_ptr, new_value);
	else if constexpr(sizeof(taType) == 4)
		previous_value = _InterlockedExchange(storage_ptr, new_value);
	else
		previous_value = _InterlockedExchange8(storage_ptr, new_value);

	return sAsValue(previous_value);
}


template <typename taType>
bool Atomic<taType>::CompareExchange(ValueType& ioExpected, ValueType inDesired)
{
	StorageType expected    = sAsStorage(ioExpected);
	StorageType desired     = sAsStorage(inDesired);
	auto        storage_ptr = (StorageType*)&mValue;
	StorageType previous_value;

	if constexpr(sizeof(taType) == 8)
		previous_value = _InterlockedCompareExchange64(storage_ptr, desired, expected);
	else if constexpr(sizeof(taType) == 4)
		previous_value = _InterlockedCompareExchange(storage_ptr, desired, expected);
	else
		previous_value = _InterlockedCompareExchange8(storage_ptr, desired, expected);

	bool exchanged = (previous_value == expected);

	if (!exchanged)
		ioExpected = sAsValue(previous_value);

	return exchanged;
}


template <typename taType>
typename Atomic<taType>::ValueType Atomic<taType>::Add(ValueType inValue, MemoryOrder inOrder) requires cIsIntegral<taType> && (!cIsSame<taType, bool>)
{
	StorageType value_to_add = sAsStorage(inValue);
	auto        storage_ptr  = (StorageType*)&mValue;
	StorageType previous_value;

	if constexpr(sizeof(taType) == 8)
		previous_value = _InterlockedExchangeAdd64(storage_ptr, value_to_add);
	else if constexpr(sizeof(taType) == 4)
		previous_value = _InterlockedExchangeAdd(storage_ptr, value_to_add);
	else
		previous_value = _InterlockedExchangeAdd8(storage_ptr, value_to_add);

	return sAsValue(previous_value);
}

template <typename taType>
typename Atomic<taType>::ValueType Atomic<taType>::Sub(ValueType inValue, MemoryOrder inOrder) requires cIsIntegral<taType> && (!cIsSame<taType, bool>)
{
	return Add(-inValue, inOrder);
}


template <typename taType>
typename Atomic<taType>::ValueType Atomic<taType>::sAsValue(StorageType inStorage)
{
	if constexpr (cIsIntegral<ValueType> || cIsEnum<ValueType>)
		return static_cast<ValueType>(inStorage);
	else if constexpr (cIsPointer<ValueType>)
		return reinterpret_cast<ValueType>(inStorage);
	else
	{
		// The only way of converting unrelated types that is non UB is memcpy.
		// This will be optimized out in practice.
		ValueType value;
		memcpy(&value, &inStorage, sizeof(inStorage));
		return value;
	}
}


template <typename taType>
typename Atomic<taType>::StorageType Atomic<taType>::sAsStorage(ValueType inValue)
{
	if constexpr (cIsIntegral<ValueType> || cIsEnum<ValueType>)
		return static_cast<StorageType>(inValue);
	else if constexpr (cIsPointer<ValueType>)
		return reinterpret_cast<StorageType>(inValue);
	else
	{
		// The only way of converting unrelated types that is non UB is memcpy.
		// This will be optimized out in practice.
		StorageType storage;
		memcpy(&storage, &inValue, sizeof(inValue));
		return storage;
	}
}

