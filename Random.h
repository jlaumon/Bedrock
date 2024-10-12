// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Ticks.h>

// Simple random function.
inline uint32 gRand32(uint32 inSeed = 0)
{
	if (inSeed == 0)
		inSeed = (uint32)gGetTickCount();

	// Equivalent to std::minstd_rand
	constexpr uint32 cMul = 48271;
	constexpr uint32 cMod = 2147483647;
	return inSeed * cMul % cMod;
}