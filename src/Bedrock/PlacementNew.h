// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/TypeTraits.h>

namespace Details {	struct PlacementNewTag {}; }
constexpr void* operator new(size_t, void* inPtr, Details::PlacementNewTag) noexcept { return inPtr; }
constexpr void  operator delete(void*, void*, Details::PlacementNewTag) noexcept {}

// Placement new
template<typename taType, typename... taArgs>
void gPlacementNew(taType& ioStorage, taArgs&&... inArgs) { new (&ioStorage, Details::PlacementNewTag{}) taType(gForward<taArgs>(inArgs)...); }
