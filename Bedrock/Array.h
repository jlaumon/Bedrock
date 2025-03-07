// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TypeTraits.h>

template <typename taType, int taSize>
struct Array
{
	using ValueType = taType;

	constexpr static int Size() { return taSize; }

	constexpr const taType* Data() const { return mData; }
	constexpr taType* Data() { return mData; }

	constexpr const taType* Begin() const { return mData; }
	constexpr const taType* End() const { return mData + taSize; }
	constexpr const taType* begin() const { return mData; }
	constexpr const taType* end() const { return mData + taSize; }
	constexpr taType* Begin() { return mData; }
	constexpr taType* End() { return mData + taSize; }
	constexpr taType* begin() { return mData; }
	constexpr taType* end() { return mData + taSize; }

	constexpr void Fill(const taType& inValue)
	{
		for (taType& value : mData)
			value = inValue;
	}
	
	constexpr const taType& operator[](int inPosition) const { gBoundsCheck(inPosition, taSize); return mData[inPosition]; }
	constexpr taType& operator[](int inPosition) { gBoundsCheck(inPosition, taSize); return mData[inPosition]; }

	taType mData[taSize];
};


// Array is a contiguous container.
template<class taType, int taSize> inline constexpr bool cIsContiguous<Array<taType, taSize>> = true;
