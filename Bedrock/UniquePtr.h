// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

// Very minimal, no deleter and no array support for now.
template <typename taType>
struct UniquePtr
{
	// Default
	constexpr UniquePtr() = default;
	constexpr ~UniquePtr()
	{
		delete mPtr;
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

		delete mPtr;
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
	constexpr 			operator taType*() const	{ return mPtr; }
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


