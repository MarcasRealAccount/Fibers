#include "Fibers/Stack.h"

namespace Fibers
{
	Stack::Stack(ECallingConvention callingConvention, std::size_t size)
	    : m_CallingConvention(CorrectCallingConvention(callingConvention)), m_Data(nullptr), m_Size(0ULL)
	{
		switch (callingConvention)
		{
		case ECallingConvention::Native: [[fallthrough]];
		case ECallingConvention::MSAbi: [[fallthrough]];
		case ECallingConvention::SYSVAbi:
			m_Size = (size + 15) / 16 * 16;
			m_Data = new std::uint8_t[m_Size];
			break;
		}
	}

	Stack::Stack(Stack&& move) noexcept
	    : m_CallingConvention(move.m_CallingConvention), m_Data(move.m_Data), m_Size(move.m_Size)
	{
		move.m_Data = nullptr;
		move.m_Size = 0ULL;
	}

	Stack& Stack::operator=(Stack&& move) noexcept
	{
		m_CallingConvention = move.m_CallingConvention;
		m_Data              = move.m_Data;
		m_Size              = move.m_Size;
		move.m_Data         = nullptr;
		move.m_Size         = 0ULL;
		return *this;
	}

	Stack::~Stack()
	{
		switch (m_CallingConvention)
		{
		case ECallingConvention::Native: [[fallthrough]];
		case ECallingConvention::MSAbi: [[fallthrough]];
		case ECallingConvention::SYSVAbi:
			if (m_Data)
				delete[] m_Data;
			m_Data = nullptr;
			m_Size = 0ULL;
			break;
		}
	}
} // namespace Fibers