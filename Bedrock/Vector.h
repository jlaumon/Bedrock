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
	Vector& operator=(Span<taOtherType> inSpan);
	
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
	constexpr int GetIndex(const taType& inElement) const;

	constexpr taType&		operator[](int inPosition)		 { gBoundsCheck(inPosition, mSize); return mData[inPosition]; }
	constexpr const taType& operator[](int inPosition) const { gBoundsCheck(inPosition, mSize); return mData[inPosition]; }

	void Clear();
	void ClearAndFreeMemory();
	void Reserve(int inCapacity);
	void Resize(int inNewSize, EResizeInit inInit = EResizeInit::ZeroInit);
	void ShrinkToFit();				// Note: Only does somethig if the allocator supports TryRealloc (eg. TempAllocator).

	void Insert(int inPosition, const taType& inValue);
	void Insert(int inPosition, taType&& inValue);
	void Insert(int inPosition, Span<const taType> inValues);
	template <typename... taArgs>
	void Emplace(int inPosition, taArgs&&... inArgs);

	void Erase(int inPosition);
	void Erase(int inPosition, int inCount);
	void SwapErase(int inPosition);

	void PushBack(const taType& inValue);
	void PushBack(taType&& inValue);
	template <typename... taArgs>
	taType& EmplaceBack(taArgs&&... inArgs);

	void PopBack();

private:
	void MoveFrom(Vector&& ioOther);
	void CopyFrom(Span<const taType> inOther);
	void Grow(int inCapacity);
	void MoveElementsForward(int inFromPosition, int inToPosition);
	void MoveElementsBackward(int inFromPosition, int inToPosition);

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
	ClearAndFreeMemory();
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
Vector<taType, taAllocator>& Vector<taType, taAllocator>::operator=(Span<taOtherType> inSpan)
{
	CopyFrom(inSpan);
	return *this;
}


template <typename taType, typename taAllocator>
constexpr int Vector<taType, taAllocator>::GetIndex(const taType& inElement) const
{
	int index = (int)(&inElement - mData);
	gBoundsCheck(index, mSize);
	return index;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::Clear()
{
	for (taType& element : *this)
		element.~taType();

	mSize = 0;
}

template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::ClearAndFreeMemory()
{
	Clear();

	if (mData != nullptr)
	{
		taAllocator::Free(mData, mCapacity);
		mData = nullptr;
	}
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::Reserve(int inCapacity)
{
	if (mCapacity >= inCapacity)
		return;

	int64 old_capacity = mCapacity;
	mCapacity = inCapacity;

	// Try to grow the allocation.
	if (taAllocator::TryRealloc(mData, old_capacity, mCapacity))
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


template <typename taType, typename taAllocator> void Vector<taType, taAllocator>::ShrinkToFit()
{
	if (mCapacity == mSize)
		return;

	if (taAllocator::TryRealloc(mData, mCapacity, mSize))
		mCapacity = mSize;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::Insert(int inPosition, const taType& inValue)
{
	gBoundsCheck(inPosition, mSize + 1);
	// Copying from self is not allowed.
	gAssert(&inValue < mData || &inValue > (mData + mCapacity));

	Grow(mSize + 1);

	// If we're not inserting at the end.
	if (inPosition != mSize)
	{
		// Move existing elements to free inPosition.
		MoveElementsForward(inPosition, inPosition + 1);

		// Copy the new element at inPosition.
		mData[inPosition] = inValue;
	}
	else
	{
		// Copy-construct the new element.
		gPlacementNew(mData[inPosition], inValue);
	}

	mSize++;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::Insert(int inPosition, taType&& inValue)
{
	gBoundsCheck(inPosition, mSize + 1);
	// Copying from self is not allowed.
	gAssert(&inValue < mData || &inValue > (mData + mCapacity));

	Grow(mSize + 1);

	// If we're not inserting at the end.
	if (inPosition != mSize)
	{
		// Move existing elements to free inPosition.
		MoveElementsForward(inPosition, inPosition + 1);

		// Move the new element at inPosition.
		mData[inPosition] = gMove(inValue);
	}
	else
	{
		// Move-construct the new element.
		gPlacementNew(mData[inPosition], gMove(inValue));
	}

	mSize++;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::Insert(int inPosition, Span<const taType> inValues)
{
	if (inValues.Empty())
		return;

	gBoundsCheck(inPosition, mSize + 1);
	// Copying from self is not allowed.
	gAssert(mData > inValues.End() || (mData + mCapacity) < inValues.Begin() || inValues.Empty());

	Grow(mSize + inValues.Size());

	// If we're not inserting at the end, move existing elements to free inPosition.
	if (inPosition != mSize)
		MoveElementsForward(inPosition, inPosition + inValues.Size());

	// Copy-assign or Copy-construct the new elements depending on if they're past the current end.
	int position = inPosition;
	for (const taType& value : inValues)
	{
		if (position < mSize)
			mData[position] = value;
		else
			gPlacementNew(mData[position], value);

		position++;
	}

	mSize += inValues.Size();
}


template <typename taType, typename taAllocator>
template <typename ... taArgs>
void Vector<taType, taAllocator>::Emplace(int inPosition, taArgs&&... inArgs)
{
	gBoundsCheck(inPosition, mSize + 1);

	Grow(mSize + 1);

	// If we're not inserting at the end.
	if (inPosition != mSize)
	{
		// Move existing elements to free inPosition.
		MoveElementsForward(inPosition, inPosition + 1);

		// Destruct the element at inPosition.
		mData[inPosition].~taType();
	}

	// Construct the new element.
	gPlacementNew(mData[inPosition], gForward<taArgs>(inArgs)...);
	mSize++;
}


template <typename taType, typename taAllocator> void Vector<taType, taAllocator>::Erase(int inPosition)
{
	Erase(inPosition, 1);
}


template <typename taType, typename taAllocator> void Vector<taType, taAllocator>::Erase(int inPosition, int inCount)
{
	gBoundsCheck(inPosition, mSize);
	gBoundsCheck(inPosition + inCount - 1, mSize);

	MoveElementsBackward(inPosition + inCount, inPosition);

	mSize -= inCount;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::SwapErase(int inPosition)
{
	gSwapErase(*this, Begin() + inPosition);
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::PushBack(const taType& inValue)
{
	// Copying from self is not allowed.
	gAssert(&inValue < mData || &inValue > (mData + mCapacity));

	Grow(mSize + 1);

	gPlacementNew(mData[mSize], inValue);
	mSize++;
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::PushBack(taType&& inValue)
{
	// Copying from self is not allowed.
	gAssert(&inValue < mData || &inValue > (mData + mCapacity));

	EmplaceBack(gMove(inValue));
}


template <typename taType, typename taAllocator>
template <typename ... taArgs>
taType& Vector<taType, taAllocator>::EmplaceBack(taArgs&&... inArgs)
{
	Grow(mSize + 1);

	taType& back = mData[mSize];

	gPlacementNew(back, gForward<taArgs>(inArgs)...);
	mSize++;

	return back;
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
	gAssert(mData > inOther.End() || (mData + mCapacity) < inOther.Begin() || inOther.Empty());

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


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::MoveElementsForward(int inFromPosition, int inToPosition)
{
	gAssert(inFromPosition < inToPosition);

	// Moving forward/right.
	// Example:
	// MoveElementsForward(1, 3):
	// | 0 | 1 | 2 | 3 | 4 | 5 |
	// | A | B | C | D | . | . |
	// | A | . | . | B | C | D |
	// C and D are move-constructed.
	// B is move-assigned.

	int num_elem_to_move = mSize - inFromPosition;
	int move_distance  = inToPosition - inFromPosition;
	gBoundsCheck(inToPosition + num_elem_to_move - 1, mCapacity);

	// First do the move constructs (into unused memory).
	for (taType* dest = mData + gMax(inToPosition, mSize), *dest_end = mData + inToPosition + num_elem_to_move; dest < dest_end; dest++)
	{
		taType* src = dest - move_distance;
		gPlacementNew(*dest, gMove(*src));
	}

	// Then do the move assigns, in reverse order since there may be overlap.
	for (taType* dest = mData + gMin(inToPosition + num_elem_to_move, mSize) - 1, *dest_end = mData + inToPosition; dest >= dest_end; dest--)
	{
		taType* src = dest - move_distance;
		*dest = gMove(*src);
	}
}


template <typename taType, typename taAllocator>
void Vector<taType, taAllocator>::MoveElementsBackward(int inFromPosition, int inToPosition)
{
	gAssert(inFromPosition > inToPosition);

	// Moving backward/left.
	// Example:
	// MoveElementsForward(3, 1):
	// | 0 | 1 | 2 | 3 | 4 | 5 |
	// | A | B | C | D | E | F |
	// | A | D | E | F | . | . |
	// D, E and F are move-assigned.
	// [4] and [5] are destructed.

	int num_elem_to_move = mSize - inFromPosition;
	int move_distance  = inFromPosition - inToPosition;
	gBoundsCheck(inToPosition, mSize + 1);

	// First do the move assignements.
	taType* dest = mData + inToPosition;
	for (taType* dest_end = dest + num_elem_to_move; dest < dest_end; dest++)
	{
		taType* src = dest + move_distance;
		*dest = gMove(*src);
	}

	// Then destruct the unused elements.
	for (taType* dest_end = mData + mSize; dest < dest_end; dest++)
	{
		dest->~taType();
	}
}

