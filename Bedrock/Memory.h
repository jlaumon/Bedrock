// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/MemoryArena.h>


// Allocate heap memory.
MemBlock gMemAlloc(int64 inSize);

// Free heap memory.
void gMemFree(MemBlock inMemory);


// Initialize temporary memory for the current thread.
void gThreadInitTempMemory(MemBlock inMemory);

// De-initialize temporary memory for the current thread.
[[nodiscard]] MemBlock gThreadExitTempMemory();

using TempMemArena = MemArena<16, 16>;
inline thread_local TempMemArena gTempMemArena;

// Allocate stack-like temporary memory.
inline MemBlock gTempMemAlloc(int64 inSize) { return gTempMemArena.Alloc(inSize); }

// Deallocate stack-like temporary memory.
inline void gTempMemFree(MemBlock inMemory) { gTempMemArena.Free(inMemory); }

// Try resizing ioMemory. Return true on success. Can fail if ioMemory isn't the last block or not enough free memory for inNewSize.
inline bool gTempMemTryRealloc(MemBlock& ioMemory, int64 inNewSize) { return gTempMemArena.TryRealloc(ioMemory, inNewSize); }

// Return true if inMemoryPtr is temp memory.
inline bool gIsTempMem(const void* inMemoryPtr) { return gTempMemArena.Owns(inMemoryPtr); }


