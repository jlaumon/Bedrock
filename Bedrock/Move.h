// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TypeTraits.h>


// Equivalent to std::move
template <typename taType>
[[nodiscard]] constexpr RemoveReference<taType>&& gMove(taType&& ioArg) { return static_cast<RemoveReference<taType>&&>(ioArg); }


/// Equivalent to std::forward
template <typename taType>
[[nodiscard]] constexpr taType&& gForward(RemoveReference<taType>& ioArg) { return static_cast<taType&&>(ioArg); }
template <typename taType>
[[nodiscard]] constexpr taType&& gForward(RemoveReference<taType>&& ioArg)
{
	static_assert(!cIsLValueReference<taType>, "Can't forward an lvalue reference.");
	return static_cast<taType&&>(ioArg);
}

