// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/PlacementNew.h>


// Holds storage for a value but does not construct it until Create() is called.
// This is functionally close to Optional<taType>, but the goal is only to delay construction, not pass around optional values.
template <typename taType>
struct Storage : NoCopy
{
	// Default
	constexpr Storage() {}
	constexpr ~Storage()
	{
		if (mCreated)
			Destroy();
	}

	// Construct the internal value.
	template<typename... taArgs>
	constexpr void Create(taArgs&&... inArgs)
	{
		gAssert(!mCreated);
		mCreated = true;
		gPlacementNew(mValue, gForward<taArgs>(inArgs)...);
	}

	// Destroy the internal value.
	void Destroy()
	{
		gAssert(mCreated);
		mCreated = false;
		mValue.~taType();
	}

	force_inline constexpr bool			 IsCreated() const				{ return mCreated; }
	force_inline constexpr taType*		 operator->()					{ gAssert(mCreated); return &mValue; }
	force_inline constexpr const taType* operator->() const				{ gAssert(mCreated); return &mValue; }
	force_inline constexpr				 operator taType*()				{ gAssert(mCreated); return &mValue; }
	force_inline constexpr				 operator const taType*() const { gAssert(mCreated); return &mValue; }

private:
	union
	{
		taType	mValue;
	};
	bool		mCreated = false;
};

