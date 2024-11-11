// SPDX-License-Identifier: MPL-2.0
#pragma once

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

// Equivalent to std::is_lvalue_reference
namespace Details
{
	template<class T> struct IsLValueReference { static constexpr bool cValue = false; };
	template<class T> struct IsLValueReference<T&> { static constexpr bool cValue = true; };
}
template<class T> constexpr bool cIsLValueReference = Details::IsConst<T>::cValue;

// Equivalent to std::is_void
template<class T> constexpr bool cIsVoid = cIsSame<void, RemoveCV<T>>;

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

