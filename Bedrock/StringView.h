// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Algorithm.h>

struct StringView
{
	constexpr StringView()								= default;
	constexpr StringView(const StringView&)				= default;
	constexpr StringView(StringView&&)					= default;
	constexpr ~StringView()								= default;
	constexpr StringView& operator=(const StringView&)	= default;
	constexpr StringView& operator=(StringView&&)		= default;
	constexpr StringView(const char* inString);
	constexpr StringView(const char* inString, int inSize);
	constexpr StringView(const char* inBegin, const char* inEnd);

	constexpr int Size() const { return mSize; }
	constexpr bool Empty() const { return mSize == 0; }
	constexpr const char* AsCStr() const;
	constexpr StringView SubStr(int inPosition, int inCount = cMaxInt) const; // Note: negative inCount behaves like cMaxInt
	constexpr void RemoveSuffix(int inCount);
	constexpr void RemovePrefix(int inCount);

	constexpr const char* Data() const { return mData; }

	constexpr const char* Begin() const { return mData; }
	constexpr const char* End() const { return mData + mSize; }
	constexpr const char* begin() const { return mData; }
	constexpr const char* end() const { return mData + mSize; }

	constexpr char Front() const { gAssert(mSize > 0); return mData[0]; }
	constexpr char Back() const { gAssert(mSize > 0); return mData[mSize - 1]; }

	constexpr bool operator==(StringView inOther) const;
	constexpr char operator[](int inPosition) const { gBoundsCheck(inPosition, mSize); return mData[inPosition]; }

	constexpr bool operator<(StringView inOther) const; // TODO: replace with <=>

	constexpr int Find(char inCharacter, int inPosition = 0) const;
	constexpr int Find(StringView inString, int inPosition = 0) const;
	constexpr int FindFirstOf(StringView inCharacters) const;
	constexpr int FindLastOf(StringView inCharacters) const;
	constexpr int FindFirstNotOf(StringView inCharacters) const;
	constexpr int FindLastNotOf(StringView inCharacters) const;

	constexpr bool Contains(StringView inString) const { return Find(inString) != -1; }
	constexpr bool Contains(char inCharacter) const { return Find(inCharacter) != -1; }

	constexpr bool StartsWith(StringView inPrefix) const;
	constexpr bool EndsWith(StringView inSuffix) const;

protected:
	static constexpr char cEmpty[1] = "";

	char*	mData		= const_cast<char*>(cEmpty); // Kept mutable only for sharing code with String.
	int		mSize		= 0;
	int		mCapacity	= 1; // Only used by String.
};


// StringView is a contiguous container.
template<> inline constexpr bool cIsContiguous<StringView> = true;

constexpr StringView::StringView(const char* inString)
{
	mData = const_cast<char*>(inString);
	mSize = gStrLen(inString);
}


constexpr StringView::StringView(const char* inString, int inSize)
{
	gAssert(inString != nullptr);
	mData = const_cast<char*>(inString);
	mSize = inSize;
}


constexpr StringView::StringView(const char* inBegin, const char* inEnd)
{
	gAssert(inBegin != nullptr && inEnd != nullptr);
	gAssert(inEnd >= inBegin);
	mData = const_cast<char*>(inBegin);
	mSize = (int)(inEnd - inBegin);
}


constexpr const char* StringView::AsCStr() const
{
	// All our strings are null terminated so it's "safe", but assert in case it's a sub-string view.
	gAssert(*End() == 0);
	return mData;
}


constexpr bool StringView::operator<(StringView inOther) const
{
	int min_size = gMin(mSize, inOther.mSize);
	int cmp      = gMemCmp(mData, inOther.mData, min_size);

	if (cmp != 0)
		return cmp < 0;
	
	return mSize < inOther.mSize;
}


constexpr int StringView::Find(char inCharacter, int inPosition) const
{
	const char* iter = gFind(Begin() + inPosition, End(), inCharacter);
	if (iter == End())
		return -1;
	else
		return (int)(iter - Begin());
}


constexpr int StringView::Find(StringView inString, int inPosition) const
{
	if (inString.Empty())
		return -1;

	const char searched_first_char = inString[0];
	const int searched_size = inString.Size();

	int pos = inPosition;
	while (true)
	{
		// Search for the first char.
		pos = Find(searched_first_char, pos);

		// If not found, the entire string will not be found.
		if (pos == -1)
			return -1;

		// If found, check if the searched string follows.
		if (SubStr(pos, searched_size) == inString)
			return pos;

		// Loop and start searching at the next char.
		pos++;
	}
}


constexpr int StringView::FindFirstOf(StringView inCharacters) const
{
	for (const char& c : *this)
	{
		if (gContains(inCharacters, c))
			return (int)(&c - mData);
	}
	return -1;
}


constexpr int StringView::FindLastOf(StringView inCharacters) const
{
	for (const char& c : gBackwards(*this))
	{
		if (gContains(inCharacters, c))
			return (int)(&c - mData);
	}
	return -1;
}


constexpr int StringView::FindFirstNotOf(StringView inCharacters) const
{
	for (const char& c : *this)
	{
		if (!gContains(inCharacters, c))
			return (int)(&c - mData);
	}
	return -1;
}


constexpr int StringView::FindLastNotOf(StringView inCharacters) const
{
	for (const char& c : gBackwards(*this))
	{
		if (!gContains(inCharacters, c))
			return (int)(&c - mData);
	}
	return -1;
}



constexpr bool StringView::StartsWith(StringView inPrefix) const
{
	return SubStr(0, inPrefix.mSize) == inPrefix;
}


constexpr bool StringView::EndsWith(StringView inSuffix) const
{
	if (mSize < inSuffix.mSize)
		return false;

	return SubStr(mSize - inSuffix.mSize, inSuffix.mSize) == inSuffix;
}


constexpr bool StringView::operator==(StringView inOther) const
{
	if (mSize != inOther.mSize)
		return false;

	return gMemCmp(mData, inOther.mData, mSize) == 0;
}


constexpr StringView StringView::SubStr(int inPosition, int inCount) const
{
	gBoundsCheck(inPosition, mSize + 1); // Note: inPosition == mSize is allowed.

	if (inCount < 0)
		inCount = cMaxInt;

	int size = gMin(inCount, mSize - inPosition);
	return { mData + inPosition, size };
}


constexpr void StringView::RemoveSuffix(int inCount)
{
	gAssert(mSize >= inCount && inCount >= 0);
	mSize -= inCount;
}


constexpr void StringView::RemovePrefix(int inCount)
{
	gAssert(mSize >= inCount && inCount >= 0);
	mData += inCount;
	mSize -= inCount;
}


uint64 gHash(StringView inValue);


template <>
struct Hash<StringView>
{
	using IsTransparent = void;

	uint64 operator()(StringView inStr) const { return gHash(inStr); }
	uint64 operator()(const char* inStr) const { return gHash(StringView(inStr)); }
};