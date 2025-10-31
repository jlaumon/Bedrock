// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Assert.h>


struct ZeroLiteral
{
	// Note: The assert is just there to break compilation if inOrder isn't 0 (because the failing case isn't constexpr code).
	consteval ZeroLiteral(int inOrder) { gAssert(inOrder == 0); }
};


#ifdef BEDROCK_ENABLE_STD

#include <compare>

#else

namespace std
{

struct partial_ordering
{
	static const partial_ordering less;
	static const partial_ordering equivalent;
	static const partial_ordering greater;
	static const partial_ordering unordered;

	friend constexpr bool operator==(partial_ordering, partial_ordering) = default;
	friend constexpr bool operator==(partial_ordering inOrder, ZeroLiteral) { return inOrder.mValue == 0; }

	friend constexpr bool operator<(partial_ordering inOrder, ZeroLiteral) { return inOrder == less; }
	friend constexpr bool operator>(partial_ordering inOrder, ZeroLiteral) { return inOrder.mValue > 0; }

	friend constexpr bool operator<=(partial_ordering inOrder, ZeroLiteral)
	{
		return inOrder == equivalent || inOrder == less;
	}
	friend constexpr bool operator>=(partial_ordering inOrder, ZeroLiteral) { return inOrder.mValue >= 0; }

	friend constexpr bool operator<(ZeroLiteral, partial_ordering inOrder) { return inOrder.mValue > 0; }
	friend constexpr bool operator>(ZeroLiteral, partial_ordering inOrder) { return inOrder == less; }

	friend constexpr bool operator<=(ZeroLiteral, partial_ordering inOrder) { return inOrder.mValue >= 0; }
	friend constexpr bool operator>=(ZeroLiteral, partial_ordering inOrder)
	{
		return inOrder == equivalent || inOrder == less;
	}

	friend constexpr partial_ordering operator<=>(partial_ordering inOrder, ZeroLiteral) { return inOrder; }
	friend constexpr partial_ordering operator<=>(ZeroLiteral, partial_ordering inOrder)
	{
		if (inOrder == less)
			return greater;

		if (inOrder == greater)
			return less;

		return inOrder;
	}

	signed char mValue;
};

inline constexpr partial_ordering partial_ordering::less	   = { -1 };
inline constexpr partial_ordering partial_ordering::equivalent = { 0 };
inline constexpr partial_ordering partial_ordering::greater	   = { 1 };
inline constexpr partial_ordering partial_ordering::unordered  = { -128 };


struct weak_ordering
{
	static const weak_ordering less;
	static const weak_ordering equivalent;
	static const weak_ordering greater;

	constexpr operator partial_ordering() const { return { mValue }; }

	friend constexpr bool operator==(weak_ordering, weak_ordering) = default;
	friend constexpr bool operator==(weak_ordering inOrder, ZeroLiteral) { return inOrder.mValue == 0; }

	friend constexpr bool operator<(weak_ordering inOrder, ZeroLiteral) { return inOrder.mValue < 0; }
	friend constexpr bool operator>(weak_ordering inOrder, ZeroLiteral) { return inOrder.mValue > 0; }

	friend constexpr bool operator<=(weak_ordering inOrder, ZeroLiteral) { return inOrder.mValue <= 0; }
	friend constexpr bool operator>=(weak_ordering inOrder, ZeroLiteral) { return inOrder.mValue >= 0; }

	friend constexpr bool operator<(ZeroLiteral, weak_ordering inOrder) { return inOrder.mValue > 0; }
	friend constexpr bool operator>(ZeroLiteral, weak_ordering inOrder) { return inOrder.mValue < 0; }

	friend constexpr bool operator<=(ZeroLiteral, weak_ordering inOrder) { return inOrder.mValue >= 0; }
	friend constexpr bool operator>=(ZeroLiteral, weak_ordering inOrder) { return inOrder.mValue <= 0; }

	friend constexpr weak_ordering operator<=>(weak_ordering inOrder, ZeroLiteral) { return inOrder; }
	friend constexpr weak_ordering operator<=>(ZeroLiteral, weak_ordering inOrder)
	{
		return { (signed char)(-inOrder.mValue) };
	}

	signed char mValue;
};

inline constexpr weak_ordering weak_ordering::less		 = { -1 };
inline constexpr weak_ordering weak_ordering::equivalent = { 0 };
inline constexpr weak_ordering weak_ordering::greater	 = { 1 };


struct strong_ordering
{
	static const strong_ordering less;
	static const strong_ordering equal;
	static const strong_ordering equivalent;
	static const strong_ordering greater;

	constexpr operator partial_ordering() const { return { mValue }; }
	constexpr operator weak_ordering() const { return { mValue }; }

	friend constexpr bool operator==(strong_ordering, strong_ordering) = default;
	friend constexpr bool operator==(strong_ordering inOrder, ZeroLiteral) { return inOrder.mValue == 0; }

	friend constexpr bool operator<(strong_ordering inOrder, ZeroLiteral) { return inOrder.mValue < 0; }
	friend constexpr bool operator>(strong_ordering inOrder, ZeroLiteral) { return inOrder.mValue > 0; }

	friend constexpr bool operator<=(strong_ordering inOrder, ZeroLiteral) { return inOrder.mValue <= 0; }
	friend constexpr bool operator>=(strong_ordering inOrder, ZeroLiteral) { return inOrder.mValue >= 0; }

	friend constexpr bool operator<(ZeroLiteral, strong_ordering inOrder) { return inOrder.mValue > 0; }
	friend constexpr bool operator>(ZeroLiteral, strong_ordering inOrder) { return inOrder.mValue < 0; }

	friend constexpr bool operator<=(ZeroLiteral, strong_ordering inOrder) { return inOrder.mValue >= 0; }
	friend constexpr bool operator>=(ZeroLiteral, strong_ordering inOrder) { return inOrder.mValue <= 0; }

	friend constexpr strong_ordering operator<=>(strong_ordering inOrder, ZeroLiteral) { return inOrder; }
	friend constexpr strong_ordering operator<=>(ZeroLiteral, strong_ordering inOrder)
	{
		return { (signed char)-inOrder.mValue };
	}

	signed char mValue;
};

inline constexpr strong_ordering strong_ordering::less		 = { -1 };
inline constexpr strong_ordering strong_ordering::equal		 = { 0 };
inline constexpr strong_ordering strong_ordering::equivalent = { 0 };
inline constexpr strong_ordering strong_ordering::greater	 = { 1 };

}

#endif

using PartialOrdering = std::partial_ordering;
using WeakOrdering	  = std::weak_ordering;
using StrongOrdering  = std::strong_ordering;
