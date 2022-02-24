#pragma once

#include "Fibers/Utils/Core.h"

namespace Fibers
{
	namespace Details
	{
		struct XMM
		{
		public:
			std::uint64_t m_L = 0, m_H = 0;
		};

		struct Registers
		{
		public:
			std::uint64_t m_RBX = 0, m_RDI = 0, m_RSI = 0, m_RBP = 0, m_RSP = 0, m_R12 = 0, m_R13 = 0, m_R14 = 0, m_R15 = 0, m_RIP = 0;
		};

		struct EntrypointRegisters
		{
		public:
			std::uint64_t m_RCX = 0, m_RDX = 0, m_R8 = 0, m_R9 = 0;
			XMM           m_XMM0, m_XMM1, m_XMM2, m_XMM3;
		};
	} // namespace Details
} // namespace Fibers