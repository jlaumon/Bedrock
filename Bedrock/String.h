// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Allocator.h>
#include <Bedrock/StringView.h>
#include <Bedrock/InitializerList.h>


template <template <typename> typename taAllocator>
struct StringBase : StringView, private taAllocator<char>
{
	using Allocator = taAllocator<char>;

	// Default
	constexpr StringBase() = default;
	~StringBase();

	// Move
	StringBase(StringBase&& ioOther);
	StringBase& operator=(StringBase&& ioOther);

	// Copy
	StringBase(const StringBase& inOther) : StringBase(static_cast<const StringView&>(inOther)) {}
	StringBase& operator=(const StringBase& inOther) { *this = static_cast<const StringView&>(inOther); return *this; }

	// Copy from String with different allocator
	template <template <typename> typename taOtherAllocator>
	requires (!cIsSame<taAllocator<char>, taOtherAllocator<char>>)
	StringBase(const StringBase<taOtherAllocator>& inOther) : StringBase(static_cast<const StringView&>(inOther)) {}
	template <template <typename> typename taOtherAllocator>
	requires (!cIsSame<taAllocator<char>, taOtherAllocator<char>>)
	StringBase& operator=(const StringBase<taOtherAllocator>& inOther) { *this = static_cast<const StringView&>(inOther); return *this; }

	// Copy from StringView
	StringBase(const StringView& inString);
	StringBase& operator=(const StringView& inString);

	// Copy from const char*
	StringBase(const char* inString) : StringBase(StringView(inString)) {}
	StringBase(const char* inString, int inSize) : StringBase(StringView(inString, inSize)) {}
	StringBase& operator=(const char* inString) { *this = StringView(inString); return *this; }

	// Copy from InitializerList (not super useful but resolves ambiguity when using = {})
	StringBase(InitializerList<char> inInitializerList);
	StringBase& operator=(InitializerList<char> inInitializerList);

	Allocator& GetAllocator() { return *this; }

	constexpr void RemoveSuffix(int inCount);

	constexpr char* Data() const { return mData; }
	constexpr char* Begin() { return mData; }
	constexpr char* End() { return mData + mSize; }
	constexpr char* begin() { return mData; }
	constexpr char* end() { return mData + mSize; }
	using StringView::Begin;
	using StringView::End;
	using StringView::begin;
	using StringView::end;

	constexpr char& Front() { gAssert(mSize > 0); return mData[0]; }
	constexpr char& Back() { gAssert(mSize > 0); return mData[mSize - 1]; }
	using StringView::Front;
	using StringView::Back;

	char& operator[](int inPosition) { gBoundsCheck(inPosition, mSize); return mData[inPosition]; }
	using StringView::operator[];

	void Reserve(int inCapacity);	// Note: inCapacity includes the null terminator.
	void Resize(int inSize);		// Note: inSize does not include the null terminator (it is stored at [Size()]).
	void Clear() { Resize(0); }
	void ShrinkToFit();				// Note: Only does somethig if the allocator supports TryRealloc (eg. TempAllocator).

	int Capacity() const { return mCapacity; }

	void Append(StringView inString);
	void Append(const char* inString, int inSize) { Append(StringView(inString, inSize)); }
	void operator+=(StringView inString) { Append(inString); }

private:
	void MoveFrom(StringBase&& ioOther);
	void CopyFrom(StringView inOther);

	using StringView::RemovePrefix; // Doesn't work on String as it just increments mData.
};


template <template <typename> typename taAllocator>
constexpr void StringBase<taAllocator>::RemoveSuffix(int inCount)
{
	gAssert(mSize >= inCount);
	mSize -= inCount;
	mData[mSize] = 0; // Re-null terminate the string.
}


static_assert(sizeof(StringBase<DefaultAllocator>) == 16);

// String is a contiguous container.
template<template <typename> typename T> inline constexpr bool cIsContiguous<StringBase<T>> = true;


using String     = StringBase<DefaultAllocator>;
using TempString = StringBase<TempAllocator>;


template <> struct Hash<String> : Hash<StringView> {};


template <template <typename> typename taAllocator>
StringBase<taAllocator>::~StringBase()
{
	if (mData != cEmpty)
		Allocator::Free(mData, mCapacity);
}


template <template <typename> typename taAllocator>
StringBase<taAllocator>::StringBase(StringBase&& ioOther)
{
	MoveFrom(gMove(ioOther));
}


template <template <typename> typename taAllocator>
StringBase<taAllocator>& StringBase<taAllocator>::operator=(StringBase&& ioOther)
{
	MoveFrom(gMove(ioOther));
	return *this;
}


template <template <typename> typename taAllocator>
StringBase<taAllocator>::StringBase(const StringView& inString)
{
	CopyFrom(inString);
}


template <template <typename> typename taAllocator>
StringBase<taAllocator>& StringBase<taAllocator>::operator=(const StringView& inString)
{
	CopyFrom(inString);
	return *this;
}


template <template <typename> typename taAllocator>
StringBase<taAllocator>::StringBase(InitializerList<char> inInitializerList)
{
	// Make sure we don't build a StringView from a nullptr.
	if (inInitializerList.size() != 0)
		CopyFrom(StringView(inInitializerList.begin(), (int)inInitializerList.size()));
}


template <template <typename> typename taAllocator>
StringBase<taAllocator>& StringBase<taAllocator>::operator=(InitializerList<char> inInitializerList)
{
	// Make sure we don't build a StringView from a nullptr.
	if (inInitializerList.size() != 0)
		CopyFrom(StringView(inInitializerList.begin(), (int)inInitializerList.size()));
	else
		Resize(0);

	return *this;
}


// Note: inCapacity includes the null terminator.
template <template <typename> typename taAllocator>
void StringBase<taAllocator>::Reserve(int inCapacity)
{
	if (mCapacity >= inCapacity)
		return; // Capacity is already enough, early out.

	int64 old_capacity = mCapacity;
	mCapacity = inCapacity;

	gAssert(inCapacity > 1); // Reserving for storing an empty string? That should not happen.

	// Try to grow the allocation.
	if (mData != nullptr && Allocator::TryRealloc(mData, old_capacity, mCapacity))
		return; // Success, nothing else to do.

	// Allocate new data.
	char* old_data = mData;
	mData = Allocator::Allocate(mCapacity);

	// Copy old data to new.
	gMemCopy(mData, old_data, mSize);
	mData[mSize] = 0;

	// Free old data.
	if (old_data != cEmpty)
		Allocator::Free(old_data, old_capacity);
}


template <template <typename> typename taAllocator>
void StringBase<taAllocator>::Resize(int inSize)
{
	if (inSize == 0 && mData == cEmpty)
		return;

	Reserve(inSize + 1);

	mData[inSize] = 0;
	mSize         = inSize;
}


template <template <typename> typename taAllocator> void StringBase<taAllocator>::ShrinkToFit()
{
	if (mData == cEmpty || mCapacity == (mSize + 1))
		return;

	if (Allocator::TryRealloc(mData, mCapacity, mSize + 1))
		mCapacity = mSize + 1;
}


template <template <typename> typename taAllocator>
void StringBase<taAllocator>::Append(StringView inString)
{
	if (inString.Empty())
		return;

	Reserve(mSize + inString.Size() + 1);

	gMemCopy(mData + mSize, inString.Data(), inString.Size());
	mSize += inString.Size();
	mData[mSize] = 0;
}


template <template <typename> typename taAllocator>
void StringBase<taAllocator>::MoveFrom(StringBase&& ioOther)
{
	if (ioOther.mData == cEmpty)
	{
		if (mData == cEmpty)
			return; // Nothing to do, moving an empty string to an empty string.

		// Moving an empty string to a non-empty string, don't actually move, keep the memory.
		mSize = 0;
		mData[mSize] = 0;
	}
	else
	{
		// Moving from self is not allowed.
		gAssert(mData != ioOther.mData);

		// Free the existing data.
		if (mData != cEmpty)
			Allocator::Free(mData, mCapacity);

		// Move the allocator.
		GetAllocator() = gMove(ioOther.GetAllocator());
		ioOther.GetAllocator() = Allocator();

		mData     = ioOther.mData;
		mSize     = ioOther.mSize;
		mCapacity = ioOther.mCapacity;

		ioOther.mData     = const_cast<char*>(cEmpty);
		ioOther.mSize     = 0;
		ioOther.mCapacity = 1;
	}
}


template <template <typename> typename taAllocator>
void StringBase<taAllocator>::CopyFrom(StringView inOther)
{
	// Copying from self is not allowed (unless it's cEmpty).
	gAssert(Begin() > inOther.End() || End() < inOther.Begin() || mData == cEmpty);

	Resize(0); // Makes sure Reserve doesn't have to copy data over to a new allocation.

	// If copying from an empty string, nothing to do.
	if (inOther.Empty())
		return;

	Reserve(inOther.Size() + 1);

	mSize = inOther.Size();

	gMemCopy(mData, inOther.Begin(), mSize);
	mData[mSize] = 0;
}


