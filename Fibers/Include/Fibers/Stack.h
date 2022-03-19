#pragma once

#include "CallingConvention.h"

#include <cstddef>

namespace Fibers
{
	struct Stack
	{
	public:
		Stack(ECallingConvention callingConvention, std::size_t size);
		Stack(Stack&& move) noexcept;
		Stack& operator=(Stack&& move) noexcept;
		~Stack();

		auto getCallingConvention() const { return m_CallingConvention; }
		auto getData() const { return m_Data; }
		auto getSize() const { return m_Size; }

		auto getTop() const { return m_Data + m_Size; }

	private:
		ECallingConvention m_CallingConvention;
		std::uint8_t*      m_Data;
		std::size_t        m_Size;
	};
} // namespace Fibers