#pragma once

#include "Fibers/Utils/Core.h"

#if BUILD_IS_SYSTEM_WINDOWS

#include "Registers.h"

extern "C" void msabiStoreFiberStates(Fibers::Details::Registers& storeRegisters, void* returnAddress);
extern "C" void msabiRestoreFiberStates(Fibers::Details::Registers& restoreRegisters, Fibers::Details::EntrypointRegisters* entrypointRegisters);
extern "C" void msabiStoreAndRestoreFiberStates(Fibers::Details::Registers& storeRegisters, void* returnAddress, Fibers::Details::Registers& restoreRegisters, Fibers::Details::EntrypointRegisters* entrypointRegisters);

namespace Fibers
{
	void storeFiberStates(Fibers::Details::Registers& storeRegisters, void* returnAddress)
	{
		msabiStoreFiberStates(storeRegisters, returnAddress);
	}

	void restoreFiberStates(Fibers::Details::Registers& restoreRegisters, Fibers::Details::EntrypointRegisters* entrypointRegisters)
	{
		msabiRestoreFiberStates(restoreRegisters, entrypointRegisters);
	}

	void storeAndRestoreFiberStates(Fibers::Details::Registers& storeRegisters, void* returnAddress, Fibers::Details::Registers& restoreRegisters, Fibers::Details::EntrypointRegisters* entrypointRegisters)
	{
		msabiStoreAndRestoreFiberStates(storeRegisters, returnAddress, restoreRegisters, entrypointRegisters);
	}
} // namespace Fibers

#endif