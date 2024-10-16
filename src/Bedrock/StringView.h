// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Algorithm.h>

class StringView
{
public:
	constexpr StringView()								= default;
	constexpr StringView(const StringView&)				= default;
	constexpr StringView(StringView&&)					= default;
	constexpr ~StringView()								= default;
	constexpr StringView& operator=(const StringView&)	= default;
	constexpr StringView& operator=(StringView&&)		= default;
	constexpr StringView(const char* inString);
	constexpr StringView(const char* inString, int inSize);
	constexpr StringView& operator=(const char* inString);

	constexpr int Size() const { return mSize; }
	constexpr bool Empty() const { return mSize == 0; }
	constexpr const char* AsCStr() const;
	constexpr StringView SubStr(int inPosition, int inCount = cMaxInt) const;

	constexpr const char* Begin() const { return mData; }
	constexpr const char* End() const { return mData + mSize; }
	constexpr const char* begin() const { return mData; }
	constexpr const char* end() const { return mData + mSize; }

	constexpr bool operator==(StringView inOther) const;
	constexpr char operator[](int inPosition) const { gBoundsCheck(inPosition, mSize); return mData[inPosition]; }

	constexpr int Find(char inCharacter) const;
	constexpr int Find(StringView inString) const;
	constexpr int FindFirstOf(StringView inCharacters) const;
	constexpr int FindLastOf(StringView inCharacters) const;

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


constexpr StringView& StringView::operator=(const char* inString)
{
	*this = StringView(inString);
	return *this;
}


constexpr const char* StringView::AsCStr() const
{
	// All our strings are null terminated so it's "safe", but assert in case it's a sub-string view.
	gAssert(*End() == 0);
	return mData;
}


constexpr int StringView::Find(char inCharacter) const
{
	const char* iter = gFind(*this, inCharacter);
	if (iter == End())
		return -1;
	else
		return (int)(iter - Begin());
}


constexpr int StringView::Find(StringView inString) const
{
	if (inString.Empty())
		return -1;

	const char searched_first_char = inString[0];
	const int searched_size = inString.Size();

	int pos = 0;
	while (true)
	{
		// Search for the first char.
		pos = SubStr(pos).Find(searched_first_char);

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


constexpr bool StringView::operator==(StringView inOther) const
{
	if (mSize != inOther.mSize)
		return false;

	return gMemCmp(mData, inOther.mData, mSize) == 0;
}


constexpr StringView StringView::SubStr(int inPosition, int inCount) const
{
	gBoundsCheck(inPosition, mSize);
	int size = gMin(inCount, mSize - inPosition);
	return { mData + inPosition, size };
}

