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
		case ECallingConvention::MSAbi:
			fibers_msabi_restore(*this, entry);
			break;
		case ECallingConvention::SYSVAbi:
			fibers_sysvabi_restore(*this, entry);
			break;
		}
	}

	void storeAndRestore(RegisterState& storeState, void* returnAddress, RegisterState& restoreState, bool entry)
	{
		fibers_storerestore(storeState, returnAddress, restoreState, entry);
	}
} // namespace Fibers