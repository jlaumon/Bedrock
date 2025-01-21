// SPDX-License-Identifier: MPL-2.0
#include <Bedrock/Compare.h>

namespace 
{
	struct CompareTest
	{
		int i, j;
		auto operator<=>(const CompareTest&) const = default;
	};

	constexpr CompareTest a = { 1, 3 };
	constexpr CompareTest b = { 3, 1 };
	constexpr CompareTest c = { 3, 1 };

	static_assert((a <=> b) < 0);
	static_assert((a <=> b) == StrongOrdering::less);

	static_assert((b <=> a) > 0);
	static_assert((b <=> a) == StrongOrdering::greater);

	static_assert((b <=> c) == 0);
	static_assert((b <=> c) == StrongOrdering::equal);
	static_assert((b <=> c) == StrongOrdering::equivalent);

	static_assert((c <=> b) == 0);
	static_assert((c <=> b) == StrongOrdering::equal);
	static_assert((c <=> b) == StrongOrdering::equivalent);

	constexpr float cNan = __builtin_nanf("");

	static_assert((1.0f <=> 3.0f) < 0);
	static_assert((1.0f <=> 3.0f) == PartialOrdering::less);

	static_assert((3.0f <=> 1.0f) > 0);
	static_assert((3.0f <=> 1.0f) > 0);
	static_assert((3.0f <=> 1.0f) == PartialOrdering::greater);

	static_assert((3.0f <=> 3.0f) == 0);
	static_assert((3.0f <=> 3.0f) == PartialOrdering::equivalent);

	static_assert((cNan <=> 1) == PartialOrdering::unordered);
	static_assert(!((cNan <=> 1) == 0));
	static_assert(!((cNan <=> 1) <= 0));
	static_assert(!((cNan <=> 1) >= 0));
	static_assert(!((cNan <=> 1) > 0));
	static_assert(!((cNan <=> 1) > 0));
	static_assert((-0.0f <=> 0.0f) == PartialOrdering::equivalent);
}
	
