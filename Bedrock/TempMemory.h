// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/MemoryArena.h>


// Initialize temporary memory for the current thread.
void gThreadInitTempMemory(MemBlock inMemory);

// De-initialize temporary memory for the current thread.
[[nodiscard]] MemBlock gThreadExitTempMemory();

// Thread-local arena that can be used for allocating temporary memory.
using TempMemArena = MemArena<>;
inline thread_local TempMemArena gTempMemArena;


