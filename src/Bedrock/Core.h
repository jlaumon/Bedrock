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


// Inherit to disallow copies.
struct NoCopy
{
	NoCopy()                         = default;
	~NoCopy()                        = default;
	NoCopy(NoCopy&&)                 = default;
	NoCopy& operator=(NoCopy&&)      = default;
	NoCopy(const NoCopy&)            = delete;
	NoCopy& operator=(const NoCopy&) = delete;
};


// Litterals for memory sizes.
constexpr size_t operator ""_B(size_t inValue)		{ return inValue; }
constexpr size_t operator ""_KiB(size_t inValue)	{ return inValue * 1024; }
constexpr size_t operator ""_MiB(size_t inValue)	{ return inValue * 1024 * 1024; }
constexpr size_t operator ""_GiB(size_t inValue)	{ return inValue * 1024 * 1024 * 1024; }


// Basic functions.
template <typename T> constexpr T gMin(T inA, T inB)				{ return inA < inB ? inA : inB; }
template <typename T> constexpr T gMax(T inA, T inB)				{ return inB < inA ? inA : inB; }
template <typename T> constexpr T gClamp(T inV, T inLow, T inHigh)	{ return (inV < inLow) ? inLow : (inHigh < inV) ? inHigh : inV; }


// Helper to get the size of C arrays.
template<typename taType, int64 taArraySize>
constexpr int64 gElemCount(const taType (&)[taArraySize]) { return taArraySize; }


// Bit twiddling. Move elsewhere?
constexpr bool  gIsPow2(int64 inValue)								{ return inValue != 0 && (inValue & (inValue - 1)) == 0; }
constexpr int64 gAlignUp(int64 inValue, int64 inPow2Alignment)		{ return (inValue + (inPow2Alignment - 1)) & ~(inPow2Alignment - 1); }
constexpr int64 gAlignDown(int64 inValue, int64 inPow2Alignment)	{ return inValue & ~(inPow2Alignment - 1); }


// Some useful C std function replacements to avoid an include or because the real ones aren't constexpr.
constexpr int gStrLen(const char* inString)									{ return (int)__builtin_strlen(inString); }
constexpr int gMemCmp(const void* inPtrA, const void* inPtrB, int inSize)	{ return __builtin_memcmp(inPtrA, inPtrB, inSize); }
extern "C" void* __cdecl memcpy(void* inDest, void const* inSource, size_t inSize);
inline void   gMemCopy(void* inDest, const void* inSource, int inSize)		{ memcpy(inDest, inSource, inSize); }

