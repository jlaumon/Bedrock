// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Test.h>
#include <Bedrock/Narrow.h>


REGISTER_TEST("Narrow")
{
	TEST_TRUE(gNarrow<int>(42) == 42);
	TEST_TRUE(gNarrow<int64>(int32(-42)) == -42);
	TEST_TRUE(gNarrow<uint64>(uint32(42)) == 42);
	TEST_TRUE(gNarrow<uint8>(uint16(255)) == 255);
	TEST_TRUE(gNarrow<int8>(int16(-128)) == -128);

	TEST_ASSERT(gNarrow<int8>(1000));
	TEST_ASSERT(gNarrow<int8>(-129));
	TEST_ASSERT(gNarrow<uint8>(-1));
	TEST_ASSERT(gNarrow<int>(1.5f));
	TEST_ASSERT(gNarrow<float>(1.1));

	TEST_TRUE(gNarrow<int>(42.0f) == 42);
	TEST_TRUE(gNarrow<float>(1.5) == 1.5f);
};
