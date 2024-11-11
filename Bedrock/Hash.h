// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

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
#include <Bedrock/../../thirdparty/rapidhash/rapidhash.h>
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

#define DECLARE_HASH(type)                                       \
	inline uint64 gHash(type inValue, uint64 inSeed = cHashSeed) \
	{                                                            \
		return gHash(&inValue, sizeof(inValue), inSeed);         \
	}                                                            \
                                                                 \
	template <> struct Hash<type>                                \
	{                                                            \
		uint64 operator()(type inValue) const                    \
		{                                                        \
			return gHash(inValue);                               \
		}                                                        \
	};


DECLARE_HASH(int8)
DECLARE_HASH(uint8)
DECLARE_HASH(int16)
DECLARE_HASH(uint16)
DECLARE_HASH(int32)
DECLARE_HASH(uint32)
DECLARE_HASH(int64)
DECLARE_HASH(uint64)
DECLARE_HASH(int)









