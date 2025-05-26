// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/PlacementNew.h>
#include <Bedrock/Move.h>

template <typename taType> struct Function;


// Polymorphic function wrapper. Roughly equivalent to std::function.
template <typename taResult, typename... taArgs>
struct Function<taResult(taArgs...)>
{
	Function() = default;
	
	~Function()
	{
		if (IsValid())
			Destruct();
	}

	// No copy for now.
	Function(const Function&) = delete;
	Function& operator=(const Function&) = delete;

	template <typename taFunc>
	requires (!cIsSame<RemoveCV<taFunc>, Function>)	// Don't accept Function types, this is not a copy-constructor.
	Function(taFunc&& ioFunc)
	{
		Construct(gForward<taFunc>(ioFunc));
	}

	Function(Function&& ioOther)
	{
		if (ioOther.IsValid())
			ioOther.MoveTo(this);
	}

	Function& operator=(Function&& ioOther)
	{
		// Moving from self is not allowed.
		gAssert(&mStorage[0] != &ioOther.mStorage[0]);

		if (IsValid())
			Destruct();

		if (ioOther.IsValid())
			ioOther.MoveTo(this);
		
		return *this;
	}

	bool IsValid() const { return mVTable != nullptr; }

	[[nodiscard]] taResult operator()(taArgs&&... ioArgs)
	{
		gAssert(IsValid());
		return mVTable->mInvoke(mStorage, gForward<taArgs>(ioArgs)...);
	}

private:
	struct VTable
	{
		using InvokeFunc    = taResult (*)(void*, taArgs&&...);
		using DestructFunc  = void (*)(void*);
		using MoveFunc      = void (*)(void*, void*);

		InvokeFunc   mInvoke   = nullptr;
		DestructFunc mDestruct = nullptr;
		MoveFunc     mMove     = nullptr;
	};

	// Contruct from a lambda.
	// TODO: add a version for function pointers.
	template <typename taLambda>
	void Construct(taLambda&& ioLambda)
	{
		static_assert(alignof(taLambda) <= cStorageAlignment);
		static_assert(sizeof(taLambda) <= cStorageSize);
		gAssert(!IsValid());

		static const VTable sVTable {
			[](void* ioStorage, taArgs&&... ioArgs)	{ return ((taLambda*)ioStorage)->operator()(gForward<taArgs>(ioArgs)...); },
			[](void* ioStorage)						{ ((taLambda*)ioStorage)->~taLambda(); },
			[](void* ioFrom, void* ioTo)			{ gPlacementNew(*(taLambda*)ioTo, gMove(*(taLambda*)ioFrom)); },
		};

		mVTable = &sVTable;
		gPlacementNew(*(taLambda*)mStorage, gForward<taLambda>(ioLambda));
	}

	void Destruct()
	{
		gAssert(IsValid());

		mVTable->mDestruct(mStorage);
		mVTable = nullptr;
	}
	
	void MoveTo(Function* ioTo)
	{
		gAssert(IsValid());
		gAssert(!ioTo->IsValid());

		mVTable->mMove(mStorage, ioTo->mStorage);
		ioTo->mVTable = mVTable;

		Destruct();
	}

	static constexpr int cStorageSize      = sizeof(void*) * 4;
	static constexpr int cStorageAlignment = alignof(void*);

	const VTable* mVTable = nullptr;
	alignas(cStorageAlignment) uint8 mStorage[cStorageSize];
};