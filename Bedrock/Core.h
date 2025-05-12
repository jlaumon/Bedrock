// SPDX-License-Identifier: MPL-2.0
#pragma once

// Assert macro.
#include <Bedrock/Assert.h>


// Basic types.
using int8   = signed char;
using uint8  = unsigned char;
using int16  = signed short;
using uint16 = unsigned short;
using int32  = signed long;
using uint32 = unsigned long;
using int64  = signed long long;
using uint64 = unsigned long long;

using NullPtrType = decltype(nullptr);

constexpr int8   cMaxInt8   = 0x7F;
constexpr uint8  cMaxUInt8  = 0xFF;
constexpr int16  cMaxInt16  = 0x7FFF;
constexpr uint16 cMaxUInt16 = 0xFFFF;
constexpr int32  cMaxInt32  = 0x7FFFFFFF;
constexpr uint32 cMaxUInt32 = 0xFFFFFFFF;
constexpr int64  cMaxInt64  = 0x7FFFFFFFFFFFFFFF;
constexpr uint64 cMaxUInt64 = 0xFFFFFFFFFFFFFFFF;

constexpr int    cMaxInt    = cMaxInt32;


// Force a function to be inlined.
// Note: With MSVC this doesn't work in non-optimized builds unless /d2Obforceinline is used.
#ifdef __clang__
#define force_inline inline __attribute__((always_inline))
#elif _MSC_VER
#define force_inline inline __forceinline
#else
#error Unknown compiler
#endif


// Force a function to not be inlined.
#ifdef __clang__
#define no_inline __attribute__ ((noinline))
#elif _MSC_VER
#define no_inline __declspec(noinline)
#else
#error Unknown compiler
#endif


// Preprocessor utilities.
#define TOKEN_PASTE1(x, y) x ## y
#define TOKEN_PASTE(x, y) TOKEN_PASTE1(x, y)


namespace Details
{
	struct DeferDummy {};
	template <class F> struct Deferrer { F f; ~Deferrer() { f(); } };
	template <class F> Deferrer<F> operator*(DeferDummy, F f) { return {f}; }
}
// Defer execution of a block of code to the end of the scope.
// eg. defer { delete ptr; };
#define defer auto TOKEN_PASTE(deferred, __LINE__) = Details::DeferDummy{} *[&]()


namespace Details
{
	struct OnceDummy {};
	template <class F> struct Initializer { Initializer(F f) { f(); } };
	template <class F> Initializer<F> operator*(OnceDummy, F f) { return {f}; }
}
// Execute a block of code only once.
// eg. do_once { printf("hello\n"); };
#define do_once static auto TOKEN_PASTE(initializer, __LINE__) = Details::OnceDummy{} *[&]()


// Inherit to disallow copies (and moves, implicitly).
struct NoCopy
{
	NoCopy()                         = default;
	~NoCopy()                        = default;
	NoCopy(const NoCopy&)            = delete;
	NoCopy& operator=(const NoCopy&) = delete;
};


// Inherit to disallow copies but allow moves.
struct MoveOnly
{
	MoveOnly()						   = default;
	~MoveOnly()						   = default;
	MoveOnly(MoveOnly&&)			   = default;
	MoveOnly& operator=(MoveOnly&&)	   = default;
	MoveOnly(const NoCopy&)			   = delete;
	MoveOnly& operator=(const NoCopy&) = delete;
};


// Litterals for memory sizes.
consteval int64 operator ""_B(unsigned long long inValue)	{ return (int64)inValue; }
consteval int64 operator ""_KiB(unsigned long long inValue)	{ return (int64)inValue * 1024; }
consteval int64 operator ""_MiB(unsigned long long inValue)	{ return (int64)inValue * 1024 * 1024; }
consteval int64 operator ""_GiB(unsigned long long inValue)	{ return (int64)inValue * 1024 * 1024 * 1024; }


// Basic functions.
template <typename T> constexpr T gMin(T inA, T inB)				{ return inA < inB ? inA : inB; }
template <typename T> constexpr T gMax(T inA, T inB)				{ return inB < inA ? inA : inB; }
template <typename T> constexpr T gClamp(T inV, T inLow, T inHigh)	{ return (inV < inLow) ? inLow : (inHigh < inV) ? inHigh : inV; }


// Helper to get the size of C arrays.
template<typename taType, int64 taArraySize>
consteval int64 gElemCount(const taType (&)[taArraySize]) { return taArraySize; }


// Return true if the calling function is evaluated in a constexpr context.
// Equivalent to std::is_constant_evaluated.
constexpr bool gIsContantEvaluated() { return __builtin_is_constant_evaluated(); }


// Bit twiddling. Move elsewhere?
constexpr bool  gIsPow2(int64 inValue)								{ return inValue != 0 && (inValue & (inValue - 1)) == 0; }
constexpr int64 gAlignUp(int64 inValue, int64 inPow2Alignment)		{ return (inValue + (inPow2Alignment - 1)) & ~(inPow2Alignment - 1); }
constexpr int64 gAlignDown(int64 inValue, int64 inPow2Alignment)	{ return inValue & ~(inPow2Alignment - 1); }


constexpr int gCountLeadingZeros64(uint64 inValue)
{
	if (gIsContantEvaluated())
	{
		int leading_zeroes = 0;
		for (; leading_zeroes < 64; leading_zeroes++)
		{
			if (inValue & ((uint64)1 << (63 - leading_zeroes)))
				break; // Found a one.
		}
		return leading_zeroes;
	}
	
	gAssert(inValue != 0);
#ifdef __clang__
	return __builtin_clzll(inValue);
#elif _MSC_VER
	unsigned char _BitScanReverse64(unsigned long* _Index, unsigned __int64 _Mask);
	uint32 index;
	_BitScanReverse64(&index, inValue);
	return 63 - index;
#else
#error Unknown compiler
#endif
}


constexpr int64 gGetNextPow2(int64 inValue)
{
	if (inValue <= 1) [[unlikely]]
		return 1;
	return (int64)1 << (64 - gCountLeadingZeros64(inValue - 1));
}


// Some useful C std function replacements to avoid an include or because the real ones aren't constexpr.
force_inline constexpr int gStrLen(const char* inString)								{ return (int)__builtin_strlen(inString); }
force_inline constexpr int gMemCmp(const void* inPtrA, const void* inPtrB, int inSize)	{ return __builtin_memcmp(inPtrA, inPtrB, inSize); }
extern "C" void* __cdecl   memcpy(void* inDest, void const* inSource, size_t inSize);
extern "C" void* __cdecl   memmove(void* inDest, void const* inSource, size_t inSize);
force_inline void		   gMemCopy(void* inDest, const void* inSource, int inSize)		{ memcpy(inDest, inSource, inSize); }
force_inline void		   gMemMove(void* inDest, const void* inSource, int inSize)		{ memmove(inDest, inSource, inSize); }


