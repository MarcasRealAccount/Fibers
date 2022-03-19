#include "Fibers/CallingConvention.h"
#include "Fibers/Utils/Core.h"

namespace Fibers
{
	ECallingConvention CorrectCallingConvention(ECallingConvention callingConvention)
	{
		if (callingConvention == ECallingConvention::Native)
		{
			if constexpr (Core::s_IsSystemWindows)
				callingConvention = ECallingConvention::MSAbi;
			else if constexpr (Core::s_IsSystemUnix)
				callingConvention = ECallingConvention::SYSVAbi;
		}
		return callingConvention;
	}
} // namespace Fibers