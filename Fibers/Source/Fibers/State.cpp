#include "Fibers/State.h"

#include "Fibers/Utils/Core.h"

extern "C"
{
	void fibers_msabi_store(Fibers::RegisterState& state, void* returnAddress);
	void fibers_msabi_restore(Fibers::RegisterState& state, bool entry);
	void fibers_sysvabi_store(Fibers::RegisterState& state, void* returnAddress);
	void fibers_sysvabi_restore(Fibers::RegisterState& state, bool entry);

	void fibers_storerestore(Fibers::RegisterState& storeState, void* returnAddress, Fibers::RegisterState& restoreState, bool entry);
}

namespace Fibers
{
	RegisterState::RegisterState(ECallingConvention callingConvention)
	    : m_CallingConvention(CorrectCallingConvention(callingConvention)) {}

	void RegisterState::store(void* returnAddress)
	{
		switch (m_CallingConvention)
		{
		case ECallingConvention::Native: [[fallthrough]];
		case ECallingConvention::MSAbi:
			fibers_msabi_store(*this, returnAddress);
			break;
		case ECallingConvention::SYSVAbi:
			fibers_sysvabi_store(*this, returnAddress);
			break;
		}
	}

	void RegisterState::restore(bool entry)
	{
		switch (m_CallingConvention)
		{
		case ECallingConvention::Native: [[fallthrough]];
		case ECallingConvention::MSAbi:
			fibers_msabi_restore(*this, entry);
			break;
		case ECallingConvention::SYSVAbi:
			fibers_sysvabi_restore(*this, entry);
			break;
		}
	}

	void RegisterState::pushb(std::uint8_t byte)
	{
		m_RSP -= 1;
		*reinterpret_cast<std::uint8_t*>(m_RSP) = byte;
	}

	void RegisterState::pushw(std::uint16_t word)
	{
		m_RSP -= 2;
		*reinterpret_cast<std::uint16_t*>(m_RSP) = word;
	}

	void RegisterState::pushd(std::uint32_t dword)
	{
		m_RSP -= 4;
		*reinterpret_cast<std::uint32_t*>(m_RSP) = dword;
	}

	void RegisterState::pushq(std::uint64_t qword)
	{
		m_RSP -= 8;
		*reinterpret_cast<std::uint64_t*>(m_RSP) = qword;
	}

	std::uint8_t RegisterState::popb()
	{
		std::uint8_t val = *reinterpret_cast<std::uint8_t*>(m_RSP);
		m_RSP += 1;
		return val;
	}

	std::uint16_t RegisterState::popw()
	{
		std::uint16_t val = *reinterpret_cast<std::uint16_t*>(m_RSP);
		m_RSP += 2;
		return val;
	}

	std::uint32_t RegisterState::popd()
	{
		std::uint32_t val = *reinterpret_cast<std::uint32_t*>(m_RSP);
		m_RSP += 4;
		return val;
	}

	std::uint64_t RegisterState::popq()
	{
		std::uint64_t val = *reinterpret_cast<std::uint64_t*>(m_RSP);
		m_RSP += 8;
		return val;
	}

	void StoreAndRestore(RegisterState& storeState, void* returnAddress, RegisterState& restoreState, bool entry)
	{
		fibers_storerestore(storeState, returnAddress, restoreState, entry);
	}
} // namespace Fibers