// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

constexpr void gDefaultDelete(auto* inValue) { delete inValue; };


// Very minimal, deleter is a function only, no array support for now.
template <typename taType, void(*taDeleter)(taType*) = gDefaultDelete<taType>>
struct UniquePtr
{
	// Default
	constexpr UniquePtr() = default;
	constexpr ~UniquePtr()
	{
		taDeleter(mPtr);
	}

	// No copy
	UniquePtr(const UniquePtr&) = delete;
	UniquePtr& operator=(const UniquePtr&) = delete;

	// Move
	constexpr UniquePtr(UniquePtr&& ioOther)
	{
		mPtr = ioOther.mPtr;
		ioOther.mPtr = nullptr;
	}
	constexpr UniquePtr& operator=(UniquePtr&& ioOther)
	{
		// Early out if moving to self
		if (this == &ioOther)
			return *this;

		taDeleter(mPtr);
		mPtr = ioOther.mPtr;
		ioOther.mPtr = nullptr;
		return *this;
	}

	// From pointer
	constexpr explicit UniquePtr(taType* inPtr) : mPtr(inPtr) {}

	// From nullptr
	constexpr UniquePtr(NullPtrType) {}

	constexpr taType*	Get() const					{ return mPtr; }
	constexpr taType*	operator->() const			{ return mPtr; }
	constexpr 			operator taType*() const&	{ return mPtr; }
	constexpr 			operator taType*() const&&	= delete; // Dangerous: The pointer would probably be deleted by the time you use it
	constexpr explicit	operator bool() const		{ return mPtr != nullptr; }

	taType* Detach() const
	{
		taType* ptr = mPtr;
		mPtr = nullptr;
		return ptr;
	}

private:
	taType* mPtr = nullptr;
};


