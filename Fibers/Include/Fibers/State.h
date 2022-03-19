#pragma once

#include "CallingConvention.h"
#include "Utils/Core.h"

namespace Fibers
{
	struct alignas(16) XMM
	{
	public:
		std::uint64_t m_L = 0, m_H = 0;
	};

	struct RegisterState
	{
	public:
		RegisterState(ECallingConvention callingConvention);

		void store(void* returnAddress = nullptr);
		void restore(bool entry = false);

		void pushb(std::uint8_t byte);
		void pushw(std::uint16_t word);
		void pushd(std::uint32_t dword);
		void pushq(std::uint64_t qword);

		std::uint8_t  popb();
		std::uint16_t popw();
		std::uint32_t popd();
		std::uint64_t popq();

		template <class T>
		std::size_t push(T&& v);
		template <class T>
		std::size_t push(T&& v, std::size_t minimumAlignment);
		template <class T>
		T pop(std::size_t alignment = 0);

	public:
		ECallingConvention m_CallingConvention;

		std::uint64_t m_RAX = 0, m_RBX = 0, m_RCX = 0, m_RDX = 0, m_RDI = 0, m_RSI = 0, m_RBP = 0, m_RSP = 0;
		std::uint64_t m_R8 = 0, m_R9 = 0, m_R10 = 0, m_R11 = 0, m_R12 = 0, m_R13 = 0, m_R14 = 0, m_R15 = 0;
		std::uint64_t m_RIP = 0;

		XMM m_XMM0, m_XMM1, m_XMM2, m_XMM3, m_XMM4, m_XMM5, m_XMM6, m_XMM7, m_XMM8, m_XMM9, m_XMM10, m_XMM11, m_XMM12, m_XMM13, m_XMM14, m_XMM15;
	};

	void StoreAndRestore(RegisterState& storeState, void* returnAddress, RegisterState& restoreState, bool entry);
} // namespace Fibers

//----------------
// Implementation
//----------------

#include "CallingConventions.h"

namespace Fibers
{
	template <class T>
	std::size_t RegisterState::push(T&& v)
	{
		std::size_t allocationSize = RequiredSize<T>(m_CallingConvention);

		constexpr std::size_t alignment = alignof(T);
		std::size_t           diff      = m_RSP;

		m_RSP = (m_RSP - allocationSize) / alignment * alignment;
		diff  = diff - m_RSP;

		*reinterpret_cast<T*>(m_RSP) = std::forward<T>(v);
		return allocationSize + diff;
	}

	template <class T>
	std::size_t RegisterState::push(T&& v, std::size_t minimumAlignment)
	{
		std::size_t allocationSize = RequiredSize<T>(m_CallingConvention);

		std::size_t alignment = std::max(minimumAlignment, alignof(T));
		std::size_t diff      = m_RSP;

		m_RSP = (m_RSP - allocationSize) / alignment * alignment;
		diff -= m_RSP;

		*reinterpret_cast<T*>(m_RSP) = std::forward<T>(v);
		return allocationSize + diff;
	}

	template <class T>
	T RegisterState::pop(std::size_t alignment)
	{
		std::size_t allocationSize = RequiredSize<T>(m_CallingConvention);

		T v = std::forward<T>(*reinterpret_cast<T*>(m_RSP));
		m_RSP += allocationSize + alignment;
		return v;
	}
} // namespace Fibers