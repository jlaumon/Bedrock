// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Core.h>

#include <string.h>


void gMemCopy(void* inDest, const void* inSource, int inSize)
{
	memcpy(inDest, inSource, inSize);
}
