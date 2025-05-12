// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Core.h>


// Tests for the constexpr code in gCountLeadingZeros64.
static_assert(gCountLeadingZeros64(0) == 64);
static_assert(gCountLeadingZeros64(1) == 63);
static_assert(gCountLeadingZeros64(cMaxUInt64) == 0);
static_assert(gCountLeadingZeros64(cMaxUInt32) == 32);

