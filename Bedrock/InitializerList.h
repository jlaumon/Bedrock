// SPDX-License-Identifier: MPL-2.0
#pragma once

#ifdef BEDROCK_ENABLE_STD

#include <initializer_list>

#else

namespace std
{
	template <typename taType>
	class initializer_list
	{
	public:
		using value_type      = taType;
		using reference       = const taType&;
		using const_reference = const taType&;
		using size_type       = size_t;

		using iterator       = const taType*;
		using const_iterator = const taType*;

		constexpr initializer_list() noexcept = default;

		constexpr initializer_list(const taType* inBegin, const taType* inEnd) noexcept
		{
			mBegin = inBegin;
			mEnd = inEnd;
		}

		constexpr const taType* begin() const noexcept { return mBegin; }
		constexpr const taType* end() const noexcept { return mEnd; }
		constexpr size_t size() const noexcept { return mEnd - mBegin; }

		constexpr const taType* Begin() const noexcept { return mBegin; }
		constexpr const taType* End() const noexcept { return mEnd; }
		constexpr size_t Size() const noexcept { return mEnd - mBegin; }

	private:
		const taType* mBegin = nullptr;
		const taType* mEnd = nullptr;
	};

	template <typename taType>
	constexpr const taType* begin(initializer_list<taType> inInitializerList) noexcept { return inInitializerList.begin(); }

	template <typename taType>
	constexpr const taType* end(initializer_list<taType> inInitializerList) noexcept { return inInitializerList.end(); }
}


#endif

template <typename taType>
using InitializerList = std::initializer_list<taType>;