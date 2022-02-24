#pragma once

#include "Fibers/Utils/Core.h"

#if !BUILD_IS_SYSTEM_WINDOWS

#include "Registers.h"

extern "C" void sysvabiStoreFiberStates(Fibers::Details::Registers& storeRegisters, void* returnAddress);
extern "C" void sysvabiRestoreFiberStates(Fibers::Details::Registers& restoreRegisters, Fibers::Details::EntrypointRegisters* entrypointRegisters);
extern "C" void sysvabiStoreAndRestoreFiberStates(Fibers::Details::Registers& storeRegisters, void* returnAddress, Fibers::Details::Registers& restoreRegisters, Fibers::Details::EntrypointRegisters* entrypointRegisters);

namespace Fibers
{
	void storeFiberStates(Fibers::Details::Registers& storeRegisters, void* returnAddress)
	{
		sysvabiStoreFiberStates(storeRegisters, returnAddress);
	}

	void restoreFiberStates(Fibers::Details::Registers& restoreRegisters, Fibers::Details::EntrypointRegisters* entrypointRegisters)
	{
		sysvabiRestoreFiberStates(restoreRegisters, entrypointRegisters);
	}

	void storeAndRestoreFiberStates(Fibers::Details::Registers& storeRegisters, void* returnAddress, Fibers::Details::Registers& restoreRegisters, Fibers::Details::EntrypointRegisters* entrypointRegisters)
	{
		sysvabiStoreAndRestoreFiberStates(storeRegisters, returnAddress, restoreRegisters, entrypointRegisters);
	}
} // namespace Fibers

#endif