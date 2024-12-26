// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TypeTraits.h>

// Let's save including intrin.h for just one function.
#if defined(_MSC_VER) && defined(_M_X64) && !defined(_M_ARM64EC)
extern "C" unsigned __int64 _umul128(unsigned __int64 _Multiplier, unsigned __int64 _Multiplicand, unsigned __int64* _HighProduct);
#pragma intrinsic(_umul128)
#endif

// We don't want the includes for the standard integer types but also we won't want conflicts if are already defined,
// so put everything inside a namespace.
namespace Details::Rapidhash
{
using uint64_t = uint64;
using uint32_t = uint32;
using uint8_t = uint8;

#define RAPIDHASH_NO_INCLUDES
#include <Bedrock/thirdparty/rapidhash/rapidhash.h>
}

constexpr uint64 cHashSeed = RAPID_SEED;

inline uint64 gHash(const void* inPtr, int inSize, uint64 inSeed = cHashSeed)
{
	return Details::Rapidhash::rapidhash_withSeed(inPtr, inSize, inSeed);
}

template <typename taType> struct Hash;

// True if this Hash type supports hashing multiple equivalent types.
// eg. Hash<StringView> is transparent and allows hashing const char* as well.
template <class taHash>
concept cIsTransparent = requires { typename taHash::IsTransparent; };

// Very fast hash for integer types. CAVEAT: returns 0 for a value of 0!
template <Integral taType> inline uint64 gHash(taType inValue)
{
	// Note: Seed is not exposed here because combining anything with 0 would return 0.
	return Details::Rapidhash::rapid_mix((uint64)inValue, cHashSeed);
}

// Slighly slower hash for integer types that allows a seed (useful to combine multiple hashes).
template <Integral taType> inline uint64 gHash(taType inValue, uint64 inSeed)
{
	return Details::Rapidhash::rapidhash_withSeed(&inValue, sizeof(inValue), inSeed);
}

// Hash struct specialization for integers. To use with HashMap/HashSet.
template <Integral taType>
struct Hash<taType>
{
	uint64 operator()(taType inValue) const
	{
		return gHash(inValue);
	}
};

// Hash struct specialization for pointers. To use with HashMap/HashSet.
template <typename taType>
struct Hash<taType*>
{
	uint64 operator()(taType* inValue) const
	{
		return gHash((uint64)inValue);
	}
};









