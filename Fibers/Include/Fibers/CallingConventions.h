#pragma once

#include "CallingConvention.h"
#include "Stack.h"
#include "State.h"

namespace Fibers
{
	template <class... Ts, std::size_t... Is>
	void PushMSAbiArguments(struct RegisterState& state, std::uintptr_t returnAddress, Ts&&... vs, const std::index_sequence<Is...>& is)
	{
	}

	template <class... Ts, std::size_t... Is>
	void PushSYSVAbiArguments(struct RegisterState& state, std::uintptr_t returnAddress, Ts&&... vs, const std::index_sequence<Is...>& is)
	{
	}

	template <class... Ts>
	void PushArguments(ECallingConvention callingConvention, struct RegisterState& state, std::uintptr_t returnAddress, Ts&&... vs)
	{
		switch (callingConvention)
		{
		case ECallingConvention::MSAbi:
			PushMSAbiArguments<Ts...>(state, returnAddress, std::forward<Ts>(vs)..., std::make_index_sequence<sizeof...(Ts)>());
			break;
		case ECallingConvention::SYSVAbi:
			PushSYSVAbiArguments<Ts...>(state, returnAddress, std::forward<Ts>(vs)..., std::make_index_sequence<sizeof...(Ts)>());
			break;
		}
	}

	template <class... Ts>
	void DestroyMSAbiArguments(struct RegisterState& state)
	{
	}

	template <class... Ts>
	void DestroySYSVAbiArguments(struct RegisterState& state)
	{
	}

	template <class... Ts>
	void DestroyArguments(ECallingConvention callingConvention, struct RegisterState& state)
	{
		switch (callingConvention)
		{
		case ECallingConvention::MSAbi:
			DestroyMSAbiArguments<Ts...>(state);
			break;
		case ECallingConvention::SYSVAbi:
			DestroySYSVAbiArguments<Ts...>(state);
			break;
		}
	}
} // namespace Fibers