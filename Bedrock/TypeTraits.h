// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>

// Equivalent to std::is_class
template<class T> constexpr bool cIsClass = __is_class(T);

// Equivalent to std::is_enum
template<class T> constexpr bool cIsEnum  = __is_enum(T);

// Equivalent to std::is_union
template<class T> constexpr bool cIsUnion = __is_union(T);

// Equivalent to std::is_move_constructible
template<class T> constexpr bool cIsMoveConstructible = __is_constructible(T, T);

// Equivalent to std::is_move_assignable
template<class T> constexpr bool cIsMoveAssignable = __is_assignable(T&, T&&);

// Equivalent to std::is_assignable
template<class T, class U> constexpr bool cIsAssignable = __is_assignable(T, U);

// Equivalent to std::is_convertible
template<class taFrom, class taTo> constexpr bool cIsConvertible = __is_convertible_to(taFrom, taTo);

// Equivalent to std::has_unique_object_representations
template<class T> constexpr bool cHasUniqueObjectRepresentations = __has_unique_object_representations(T);

// Equivalent to std::is_trivially_default_constructible
template <class T> constexpr bool cIsTriviallyDefaultConstructible = __is_trivially_constructible(T);

// Equivalent to std::is_trivially_copyable
template <class T> constexpr bool cIsTriviallyCopyable = __is_trivially_copyable(T);

// Equivalent to std::is_const
namespace Details
{
	template<class T> struct IsConst { static constexpr bool cValue = false; };
	template<class T> struct IsConst<const T> { static constexpr bool cValue = true; };
}
template<class T> constexpr bool cIsConst = Details::IsConst<T>::cValue;

// Equivalent to std::is_same
namespace Details
{
	template<class T, class U> struct IsSame { static constexpr bool cValue = false; };
	template<class T> struct IsSame<T, T> { static constexpr bool cValue = true; };
}
template<class T, class U> constexpr bool cIsSame = Details::IsSame<T, U>::cValue;

// True if T is any of the types in taTypes
template<class T, class... taTypes> constexpr bool cIsAnyOf = (cIsSame<T, taTypes> || ...);

// Equivalent to std::remove_reference
namespace Details
{
	template<class T> struct RemoveReference { using Type = T; };
	template<class T> struct RemoveReference<T&> { using Type = T; };
	template<class T> struct RemoveReference<T&&> { using Type = T; };
}
template<class T> using RemoveReference = typename Details::RemoveReference<T>::Type;

// Equivalent to std::remove_cv
namespace Details
{
	template<class T> struct RemoveCV { using Type = T; };
	template<class T> struct RemoveCV<const T> { using Type = T; };
	template<class T> struct RemoveCV<volatile T> { using Type = T; };
	template<class T> struct RemoveCV<const volatile T> { using Type = T; };
}
template<class T> using RemoveCV = typename Details::RemoveCV<T>::Type;

// Equivalent to std::remove_pointer
namespace Details
{
	template<class T> struct RemovePointer { using Type = T; };
	template<class T> struct RemovePointer<T*> { using Type = T; };
	template<class T> struct RemovePointer<T* const> { using Type = T; };
	template<class T> struct RemovePointer<T* volatile> { using Type = T; };
	template<class T> struct RemovePointer<T* const volatile> { using Type = T; };
}
template<class T> using RemovePointer = typename Details::RemovePointer<T>::Type;

// Equivalent to std::is_lvalue_reference
namespace Details
{
	template<class T> struct IsLValueReference { static constexpr bool cValue = false; };
	template<class T> struct IsLValueReference<T&> { static constexpr bool cValue = true; };
}
template<class T> constexpr bool cIsLValueReference = Details::IsConst<T>::cValue;

// Equivalent to std::is_void
template<class T> constexpr bool cIsVoid = cIsSame<void, RemoveCV<T>>;

// Equivalent to std::is_pointer
namespace Details
{
	template<class T> struct IsPointer	{ static constexpr bool cValue = false; };
	template<class T> struct IsPointer<T*> { static constexpr bool cValue = true; };
	template<class T> struct IsPointer<T* const> { static constexpr bool cValue = true; };
	template<class T> struct IsPointer<T* volatile> { static constexpr bool cValue = true; };
	template<class T> struct IsPointer<T* const volatile> { static constexpr bool cValue = true; };
}
template<class T> constexpr bool cIsPointer = Details::IsPointer<T>::cValue;

// Equivalent to std::conditional
namespace Details
{
	template<bool, class T, class F> struct Conditional { using Type = T; };
	template<class T, class F> struct Conditional<false, T, F> { using Type = F; };
}
template<bool B, class T, class F> using Conditional = typename Details::Conditional<B, T, F>::Type;

// True if the elements of a container are contiguous in memory.
// Each contiguous container needs to add its specialization.
template<class T> constexpr bool cIsContiguous = false;

// True if the elements of a container stay at the same address when it grows.
// Each contiguous container needs to add its specialization.
template<class T> constexpr bool cIsStable = false;

// Equivalent to std::underlying_type
template<class T> requires cIsEnum<T> using UnderlyingType = __underlying_type(T);

// Equivalent to std::to_underlying
template<class taEnum> requires cIsEnum<taEnum>
[[nodiscard]] ATTRIBUTE_INTRINSIC constexpr auto gToUnderlying(taEnum inEnum) { return static_cast<UnderlyingType<taEnum>>(inEnum); }

// Equivalent to std::is_integral
template<class T> constexpr bool cIsIntegral = cIsAnyOf<RemoveCV<T>, bool, char, signed char, unsigned char, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;

// Equivalent to std::integral
template <class T> concept Integral = cIsIntegral<T>;

// Equivalent to std::as_const
template <class T>
[[nodiscard]] ATTRIBUTE_INTRINSIC constexpr const T& gAsConst(T& inValue) { return inValue; }
 