// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/TypeTraits.h>

namespace 
{
struct A;
}

static_assert(cIsSigned<A> == false);
static_assert(cIsSigned<bool> == false);
static_assert(cIsSigned<volatile bool> == false);
static_assert(cIsSigned<uint32> == false);
static_assert(cIsSigned<int> == true);
static_assert(cIsSigned<float> == true);
static_assert(cIsSigned<double> == true);
