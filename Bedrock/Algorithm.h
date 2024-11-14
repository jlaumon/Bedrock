// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Move.h>


// Equivalent to std::swap
template <typename taType>
constexpr void gSwap(taType& ioA, taType& ioB)
{
	taType temp = gMove(ioA);
	ioA = gMove(ioB);
	ioB = gMove(temp);
}


// Forward declaration of the Hash struct.
// Equivalent to std::hash.
template <typename taType> struct Hash;


// Minimal reverse iterator.
template <typename taIter>
struct ReverseIterator
{
	constexpr ReverseIterator& operator++() { mIter--; return *this; }
	constexpr auto& operator*() const { return *(mIter - 1); }
	constexpr bool operator==(const ReverseIterator&) const = default;

	taIter mIter = {};
};


// Helper type to iterate backwards on a container.
template <typename taContainer>
struct ReverseAdapter
{
	constexpr auto begin() { return ReverseIterator{ mContainer.end() }; }
	constexpr auto end() { return ReverseIterator{ mContainer.begin() }; }

	taContainer& mContainer;
};


// Helper function to iterate backwards on a container.
template <typename taContainer>
constexpr ReverseAdapter<taContainer> gBackwards(taContainer& ioContainer) { return { ioContainer }; }


// Check if a value is present in a vector-like container.
template<typename taValue, typename taContainer>
constexpr bool gContains(const taContainer& ioContainer, const taValue& inElem)
{
	for (auto& elem : ioContainer)
		if (elem == inElem)
			return true;

	return false;
}


// Check if any element in the container matches the predicate.
template <typename taContainer, typename taPredicate>
bool gAnyOf(taContainer& inContainer, const taPredicate& inPredicate)
{
	for (auto& element : inContainer)
		if (inPredicate(element))
			return true;
	return false;
}


// Check if none of the elements in the container matches the predicate.
template <typename taContainer, typename taPredicate>
bool gNoneOf(taContainer& inContainer, const taPredicate& inPredicate)
{
	for (auto& element : inContainer)
		if (inPredicate(element))
			return false;
	return true;
}


// Check if all the elements in the container matches the predicate.
template <typename taContainer, typename taPredicate>
bool gAllOf(taContainer& inContainer, const taPredicate& inPredicate)
{
	for (auto& element : inContainer)
		if (!inPredicate(element))
			return false;
	return true;
}


// Check if all the elements inside two containers are equal.
template<typename taContainerA, typename taContainerB>
constexpr bool gEquals(const taContainerA& inContainerA, const taContainerB& inContainerB)
{
	if (inContainerA.Size() != inContainerB.Size())
		return false;

	auto it_a = inContainerA.Begin();
	auto it_b = inContainerB.Begin();
	auto end_a = inContainerA.End();
	while (it_a != end_a)
	{
		if (*it_a != *it_b)
			return false;

		++it_a;
		++it_b;
	}

	return true;
}


// Lower bound implementation to avoid <algorithm>
template<typename taIterator, typename taValue>
constexpr taIterator gLowerBound(taIterator inFirst, taIterator inLast, const taValue& inElem)
{
	auto first = inFirst;
	auto count = inLast - first;

    while (count > 0) 
	{
		auto count2 = count / 2;
		auto mid    = first + count2;

		if (*mid < inElem)
		{
			first = mid + 1;
			count -= count2 + 1;
		}
		else
		{
			count = count2;
		}
    }

	return first;
}


// Find a value in a sorted vector-like container.
template<typename taValue, typename taContainer>
constexpr auto gFindSorted(taContainer& inContainer, const taValue& inElem)
{
	auto end = inContainer.End();
	auto it = gLowerBound(inContainer.Begin(), end, inElem);

	if (it != end && *it == inElem)
		return it;
	else
		return end;
}


// Insert a value in a sorted vector-like container.
template<typename taValue, typename taContainer>
constexpr auto gEmplaceSorted(taContainer& ioContainer, const taValue& inElem)
{
	auto end = ioContainer.End();
	auto it = gLowerBound(ioContainer.Begin(), end, inElem);

	if (it != end && *it == inElem)
		return it;
	else
		return ioContainer.Emplace(it, inElem);
}


// Find a value in a vector-like container.
template<typename taValue, typename taContainer>
constexpr auto gFind(taContainer& inContainer, const taValue& inElem)
{
	auto end = inContainer.End();
	auto begin = inContainer.Begin();

	for (auto it = begin; it != end; ++it)
	{
		if (*it == inElem)
			return it;
	}

	return end;
}


// Erase an element from a vector-like container by swapping it with the last one and reducing the size by 1.
template<typename taContainer, typename taIterator>
constexpr void gSwapErase(taContainer& inContainer, const taIterator& inIterator)
{
	if (inIterator != (inContainer.End() - 1))
		gSwap(inContainer.Back(), *inIterator);
	inContainer.PopBack();
}


// Remove the first value that matches predicate from a vector-like container.
template<typename taContainer, typename taPredicate>
constexpr bool gSwapEraseFirstIf(taContainer& inContainer, const taPredicate& inPredicate)
{
	auto end = inContainer.End();
	auto begin = inContainer.Begin();

	for (auto it = begin; it != end; ++it)
	{
		if (inPredicate(*it))
		{
			gSwapErase(inContainer, it);
			return true;	
		}
	}

	return false;
}
