// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TypeTraits.h>

// We want gMove/gForward to be always inlined, even when optimizations are disabled.
// __forceinline doesn't work in debug in MSVC, but [[msvc::intrinsic]] does (it only works in very specific conditions though).
#ifdef __clang__
#define ATTRIBUTE_INTRINSIC __forceinline
#elif _MSC_VER
#define ATTRIBUTE_INTRINSIC [[msvc::intrinsic]]
#else
#define ATTRIBUTE_INTRINSIC
#endif


// Equivalent to std::move
template <typename taType>
[[nodiscard]] ATTRIBUTE_INTRINSIC constexpr RemoveReference<taType>&& gMove(taType&& ioArg) { return static_cast<RemoveReference<taType>&&>(ioArg); }


/// Equivalent to std::forward
template <typename taType>
[[nodiscard]] ATTRIBUTE_INTRINSIC constexpr taType&& gForward(RemoveReference<taType>& ioArg) { return static_cast<taType&&>(ioArg); }
template <typename taType>
[[nodiscard]] ATTRIBUTE_INTRINSIC constexpr taType&& gForward(RemoveReference<taType>&& ioArg)
{
	static_assert(!cIsLValueReference<taType>, "Can't forward an lvalue reference.");
	return static_cast<taType&&>(ioArg);
}


#undef ATTRIBUTE_INTRINSIC