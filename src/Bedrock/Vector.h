// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TypeTraits.h>
#include <Bedrock/Algorithm.h>
#include <Bedrock/InitializerList.h>
#include <Bedrock/Allocator.h>
#include <Bedrock/Span.h>
#include <Bedrock/PlacementNew.h>

enum class EResizeInit
{
	NoZeroInit,	// If the type has a constructor, it will be called. Otherwise it is left uninitialized.
	ZeroInit,	// If the type has a constructor, it will be called. Otherwise it is zero-initialized.
};

template <typename taType, typename taAllocator = Allocator<taType>>
struct Vector : private taAllocator
{
	static_assert(!cIsConst<taType>);

	using ValueType = taType;

	// Default
	constexpr Vector() = default;
	~Vector();

	// Move
	Vector(Vector&& ioOther);
	Vector& operator=(Vector&& ioOther);

	// Copy
	Vector(const Vector& inOther);
	Vector& operator=(const Vector& inOther);

	// Copy from Vector with different allocator
	template <class taOtherAllocator>
	requires (!cIsSame<taAllocator, taOtherAllocator>)
	Vector(const Vector<taType, taOtherAllocator>& inOther);
	template <class taOtherAllocator>
	requires (!cIsSame<taAllocator, taOtherAllocator>)
	Vector& operator=(const Vector<taOtherAllocator>& inOther);

	// Copy from InitializerList
	Vector(InitializerList<taType> inInitializerList);
	Vector& operator=(InitializerList<taType> inInitializerList);

	// Copy from Span
	template <class taOtherType>
	requires cIsConvertible<taOtherType, taType>
	Vector(Span<taOtherType> inSpan);
	template <class taOtherType>
	requires cIsConvertible<taOtherType, taType>
	Vector& operator=(Span<taType> inSpan);
	
	int Size() const { return mSize; }
	int Capacity() const { return mCapacity; }
	bool Empty() const { return mSize == 0; }

	constexpr taType* Begin() { return mData; }
	constexpr taType* End()   { return mData + mSize; }
	constexpr taType* begin() { return mData; }
	constexpr taType* end()   { return mData + mSize; }
	constexpr const taType* Begin() const { return mData; }
	constexpr const taType* End()   const { return mData + mSize; }
	constexpr const taType* begin() const { return mData; }
	constexpr const taType* end()   const { return mData + mSize; }

	constexpr const taType& Front() const { return operator[](0); }
	constexpr const taType& Back()  const { return operator[](mSize - 1); }
	constexpr taType& Front() { return operator[](0); }
	constexpr taType& Back()  { return operator[](mSize - 1); }

	constexpr taType&		operator[](int inPosition)		 { gBoundsCheck(inPosition, mSize); return mData[inPosition]; }
	constexpr const taType& operator[](int inPosition) const { gBoundsCheck(inPosition, mSize); return mData[inPosition]; }

	void Clear();
	void Reserve(int inCapacity);
	void Resize(int inNewSize, EResizeInit inInit = EResizeInit::ZeroInit);

	// TODO
	// insert
	// emplace
	// erase
	void SwapErase(int inIndex);

	void PushBack(const taType& inValue);
	void PushBack(taType&& inValue);
	template <typename... taArgs>
	void EmplaceBack(taArgs&&... inArgs);

	void PopBack();

private:
	void MoveFrom(Vector&& ioOther);
	void CopyFrom(Span<const taType> inOther);
	void Grow(int inCapacity);

	taType* mData     = nullptr;
	int     mSize     = 0;
	int     mCapacity = 0;
};


// Vector is a contiguous container.
template<class taType, class taAllocator> inline constexpr bool cIsContiguous<Vector<taType, taAllocator>> = true;


// Alias for a Vector using the TempAllocator.
template <typename taType>
using TempVector = Vector<taType, TempAllocator<taType>>;


// Deduction guides for Span.
template<typename taType, typename taAllocator>
Span(Vector<taType, taAllocator>&) -> Span<taType>;
template<typename taType, typename taAllocator>
Span(const Vector<taType, taAllocator>&) -> Span<const taType>;


template <typename taType, typename taAllocator>
Vector<taType, taAllocator>::~Vector()
{
	Clear();

	if (mData != nullptr)
		taAllocator::Free(mData, mCapacity);
}


template <typename taType, typename taAllocator>
Vector<taType, taAllocator>::Vector(Vector&& ioOther)
{
	MoveFrom(gMove(ioOther));
}


template <typename taType, typename taAllocator>
Vector<taType, taAllocator>& Vector<taType, taAllocator>::operator=(Vector&& ioOther)
{
	MoveFrom(gMove(ioOther));
	return *this;
}


template <typename taType, typename taAllocator>
Vector<taType, taAllocator>::Vector(const Vector& inOther)
{
	CopyFrom(Span(inOther));
}


template <typename taType, typename taAllocator>
Vector<taType, taAllocator>& Vector<taType, taAllocator>::operator=(const Vector& inOther)
{
	CopyFrom(Span(inOther));
	return *this;
}


template <typename taType, typename taAllocator>
template <class taOtherAllocator> requires (!cIsSame<taAllocator, taOtherAllocator>)
Vector<taType, taAllocator>::Vector(const Vector<taType, taOtherAllocator>& inOther)
{
	CopyFrom(Span(inOther));
}


template <typename taType, typename taAllocator>
template <class taOtherAllocator> requires (!cIsSame<taAllocator, taOtherAllocator>)
Vector<taType, taAllocator>& Vector<taType, taAllocator>::operator=(const Vector<taOtherAllocator>& inOther)
{
	CopyFrom(Span(inOther));
	return *this;
}


template <typename taType, typename taAllocator>
Vector<taType, taAllocator>::Vector(InitializerList<taType> inInitializerList)
{
	CopyFrom(Span(inInitializerList.begin(), (int)inInitializerList.size()));
}


template <typename taType, typename taAllocator>
Vector<taType, taAllocator>& Vector<taType, taAllocator>::operator=(InitializerList<taType> inInitializerList)
{
	CopyFrom(Span(inInitializerList.begin(), (int)inInitializerList.size()));
	return *this;
}


template <typename taType, typename taAllocator>
template <class taOtherType> requires cIsConvertible<taOtherType, taType>
Vector<taType, taAllocator>::Vector(Span<taOtherType> inSpan)
{
	CopyFrom(inSpan);
}


template <typename taType, typename taAllocator>
template <class taOtherType> requires cIsConvertible<taOtherType, taType>
Vector<taType, taAllocator>& Vector<taType, taAllocator>::operator=(Span<taType> inSpan)
{
	CopyFrom(inSpan);
	return *this;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::Clear()
{
	for (taType& element : *this)
		element.~taType();

	mSize = 0;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::Reserve(int inCapacity)
{
	if (mCapacity >= inCapacity)
		return;

	int64 old_capacity = mCapacity;
	mCapacity = inCapacity;

	// Try to grow the allocation.
	if (taAllocator::TryGrow(mData, old_capacity, mCapacity))
		return; // Success, nothing else to do.

	// Allocate new data.
	taType* old_data = mData;
	mData = taAllocator::Allocate(mCapacity);

	if constexpr (cIsMoveConstructible<taType>)
	{
		// Move old data to new.
		for (int i = 0, n = mSize; i < n; ++i)
			gPlacementNew(mData[i], gMove(old_data[i]));
	}
	else
	{
		// Copy old data to new.
		for (int i = 0, n = mSize; i < n; ++i)
			gPlacementNew(mData[i], old_data[i]);
	}

	// Free old data.
	if (old_data != nullptr)
		taAllocator::Free(old_data, old_capacity);
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::Resize(int inNewSize, EResizeInit inInit)
{
	if (inNewSize < mSize)
	{
		// Shriking.
		// Destroy the elements that need to be removed.
		for (int i = inNewSize, n = mSize; i < n; i++)
			mData[i].~taType();

		// Update the size.
		mSize = inNewSize;
	}
	else if (inNewSize > mSize)
	{
		// Growing.
		// Reserve memory first.
		Reserve(inNewSize);

		// Construct the elements.
		if (inInit == EResizeInit::ZeroInit)
		{
			for (int i = mSize, n = inNewSize; i < n; i++)
				gPlacementNew(mData[i]);
		}
		else
		{
			for (int i = mSize, n = inNewSize; i < n; i++)
				gPlacementNewNoZeroInit(mData[i]);
		}

		// Update the size.
		mSize = inNewSize;
	}
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::SwapErase(int inIndex)
{
	gSwapErase(*this, Begin() + inIndex);
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::PushBack(const taType& inValue)
{
	Grow(mSize + 1);

	gPlacementNew(mData[mSize], inValue);
	mSize++;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::PushBack(taType&& inValue)
{
	Grow(mSize + 1);

	gPlacementNew(mData[mSize], gMove(inValue));
	mSize++;
}


template <typename taType, typename taAllocator>
template <typename ... taArgs>
void Vector<taType, taAllocator>::EmplaceBack(taArgs&&... inArgs)
{
	Grow(mSize + 1);

	gPlacementNew(mData[mSize], gForward<taArgs>(inArgs)...);
	mSize++;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::PopBack()
{
	gAssert(Size() >= 1);
	mSize--;
	mData[mSize].~taType();
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::MoveFrom(Vector&& ioOther)
{
	// Moving from self is not allowed.
	gAssert(mData != ioOther.mData);

	mData     = ioOther.mData;
	mSize     = ioOther.mSize;
	mCapacity = ioOther.mCapacity;

	ioOther.mData     = nullptr;
	ioOther.mSize     = 0;
	ioOther.mCapacity = 0;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::CopyFrom(Span<const taType> inOther)
{
	// Copying from self is not allowed.
	gAssert(Begin() > inOther.End() || End() < inOther.Begin() || inOther.Empty());

	Clear();
	Reserve(inOther.Size());
	mSize = inOther.Size();

	taType* data_ptr = mData;
	for (const taType& element : inOther)
	{
		gPlacementNew(*data_ptr, element);
		data_ptr++;
	}
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::Grow(int inCapacity)
{
	if (mCapacity >= inCapacity)
		return;

	Reserve(gMax(mCapacity + mCapacity / 2, inCapacity));
}
