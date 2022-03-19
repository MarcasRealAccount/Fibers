#pragma once

#include <cstdint>

namespace Fibers
{
	enum class ECallingConvention : std::uint32_t
	{
		Native = 0U,
		MSAbi,
		SYSVAbi
	};

	ECallingConvention CorrectCallingConvention(ECallingConvention callingConvention);
}