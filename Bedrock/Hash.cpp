// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Hash.h>
#include<Bedrock/Test.h>
#include<Bedrock/StringView.h>


inline uint64 gHash(StringView inValue, uint64 inSeed = cHashSeed)
{
	return gHash(inValue.Begin(), inValue.Size(), inSeed);
}


REGISTER_TEST("Hash")
{
	uint64 hash = gHash(42);
	TEST_TRUE(hash != 0);

	uint64 hash2 = gHash(StringView("hello what's up"), hash);
	TEST_TRUE(hash2 != hash);
};
