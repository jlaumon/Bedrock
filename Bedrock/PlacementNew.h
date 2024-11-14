// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Move.h>

namespace Details {	struct PlacementNewTag {}; }
constexpr void* operator new(size_t, void* inPtr, Details::PlacementNewTag) noexcept { return inPtr; }
constexpr void  operator delete(void*, void*, Details::PlacementNewTag) noexcept {}

// Placement new
template<typename taType, typename... taArgs>
inline void gPlacementNew(taType& ioStorage, taArgs&&... inArgs) { new (&ioStorage, Details::PlacementNewTag{}) taType(gForward<taArgs>(inArgs)...); }

// Placement new that calls the default constructor only if there's one (no zero-initialization).
template<typename taType>
inline void gPlacementNewNoZeroInit(taType& ioStorage) { new (&ioStorage, Details::PlacementNewTag{}) taType; }
