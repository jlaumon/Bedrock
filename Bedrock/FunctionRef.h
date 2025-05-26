// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TypeTraits.h>
#include <Bedrock/Move.h>

template <typename taType> struct FunctionRef;

// A non-owning polymorphic function wrapper. Roughly equivalent to std::function_ref.
// Unlike Function, it doesn't store a copy of the lambda, only a pointer to it.
template <typename taResult, typename... taArgs>
struct FunctionRef<taResult(taArgs...)>
{
	// Default
	FunctionRef() = default;
	~FunctionRef() = default;

	// Copy
	FunctionRef(const FunctionRef&) = default;
	FunctionRef& operator=(const FunctionRef&) = default;

	template <typename taLambda>
	requires (!cIsSame<RemoveCV<taLambda>, FunctionRef>)	// Don't accept FunctionRef types, this is not a copy-constructor.
	FunctionRef(taLambda&& ioLambda ATTRIBUTE_LIFETIMEBOUND)
	{
		mPointer = (void*)&ioLambda;
		mInvoke	= [](void* ioPointer, taArgs&&... ioArgs) { return (*(RemoveReference<taLambda>*)ioPointer)(gForward<taArgs>(ioArgs)...); };
	}

	bool IsValid() const { return mInvoke != nullptr; }

	[[nodiscard]] taResult operator()(taArgs&&... ioArgs)
	{
		gAssert(IsValid());
		return mInvoke(mPointer, gForward<taArgs>(ioArgs)...);
	}

private:
	using InvokeFunc    = taResult (*)(void*, taArgs&&...);

	void*	   mPointer = nullptr;
	InvokeFunc mInvoke	= nullptr;
};
