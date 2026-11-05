// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TypeTraits.h>


// Narrowing conversion. Does not check anything, it's only to make it explicit to the reader.
// Equivalent to gsl::narrow_cast
template <Arithmetic taTarget, Arithmetic taSource>
[[nodiscard]] ATTRIBUTE_INTRINSIC constexpr taTarget gNarrowCast(taSource inValue) noexcept
{
	return static_cast<taTarget>(inValue);
}


// Checked narrowing conversion. Asserts if not possible without loosing information.
// Equivalent to gsl::narrow.
template <Arithmetic taTarget, Arithmetic taSource>
[[nodiscard]] constexpr taTarget gNarrow(taSource inValue) noexcept
{
	const taTarget result = gNarrowCast<taTarget>(inValue);
	const bool sign_preserved = !cIsSigned<taSource> || cIsSigned<taTarget> || inValue >= 0;
	gAssert(sign_preserved && static_cast<taSource>(result) == inValue);
	return result;
}
