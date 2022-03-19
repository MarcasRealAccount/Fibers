#pragma once

#include "CallingConvention.h"
#include "Stack.h"
#include "State.h"

#include <vector>

namespace Fibers
{
	namespace MSAbi
	{
		template <class T, std::size_t I>
		void AddStackAddress(std::vector<std::uintptr_t>& arguments, RegisterState& state, T&& v)
		{
			if constexpr (sizeof(T) > 8)
			{
				state.push<T>(std::forward<T>(v), 16);
				arguments[I + 1] = state.m_RSP;
			}
		}

		template <std::size_t I, class T, class... Ts>
		void PushArgument(std::vector<std::uintptr_t>& arguments, RegisterState& state, T&& v, Ts&&... vs)
		{
			if constexpr (sizeof...(Ts) > 0)
				PushArgument<I + 1, Ts...>(arguments, state, std::forward<Ts>(vs)...);

			if (arguments[I + 1])
			{
				if constexpr (I == 0)
					state.m_RCX = arguments[I + 1];
				else if constexpr (I == 1)
					state.m_RDX = arguments[I + 1];
				else if constexpr (I == 2)
					state.m_R8 = arguments[I + 1];
				else if constexpr (I == 3)
					state.m_R9 = arguments[I + 1];
				else
					state.pushq(arguments[I + 1]);
			}
			else
			{
				if constexpr (std::is_floating_point_v<T>)
				{
					if constexpr (I == 0)
					{
						*reinterpret_cast<T*>(&state.m_XMM0) = std::forward<T>(v);
					}
					else if constexpr (I == 1)
					{
						*reinterpret_cast<T*>(&state.m_XMM1) = std::forward<T>(v);
					}
					else if constexpr (I == 2)
					{
						*reinterpret_cast<T*>(&state.m_XMM2) = std::forward<T>(v);
					}
					else if constexpr (I == 3)
					{
						*reinterpret_cast<T*>(&state.m_XMM3) = std::forward<T>(v);
					}
					else
					{
						state.push<T>(std::forward<T>(v));
						arguments[I + 1] = state.m_RSP;
					}
				}
				else
				{
					if constexpr (I == 0)
					{
						*reinterpret_cast<T*>(&state.m_RCX) = std::forward<T>(v);
					}
					else if constexpr (I == 1)
					{
						*reinterpret_cast<T*>(&state.m_RDX) = std::forward<T>(v);
					}
					else if constexpr (I == 2)
					{
						*reinterpret_cast<T*>(&state.m_R8) = std::forward<T>(v);
					}
					else if constexpr (I == 3)
					{
						*reinterpret_cast<T*>(&state.m_R9) = std::forward<T>(v);
					}
					else
					{
						state.push<T>(std::forward<T>(v));
						arguments[I + 1] = state.m_RSP;
					}
				}
			}
		}

		template <class... Ts, std::size_t... Is>
		void PushArguments(RegisterState& state, std::vector<std::uintptr_t>& arguments, std::uintptr_t returnAddress, Ts&&... vs, const std::index_sequence<Is...>&)
		{
			arguments.resize(sizeof...(Ts) + 1);
			arguments[0] = state.m_RSP;
			if constexpr (sizeof...(Ts) > 0)
			{
				(AddStackAddress<Ts, Is>(arguments, state, std::forward<Ts>(vs)), ...);

				PushArgument<0, Ts...>(arguments, state, std::forward<Ts>(vs)...);
			}
			state.m_RSP -= 32;
			state.pushq(returnAddress);
		}

		template <class T, std::size_t I>
		void DestroyArgument([[maybe_unused]] RegisterState& state, const std::vector<std::uintptr_t>& arguments)
		{
			using TI = std::remove_reference_t<std::remove_pointer_t<T>>;

			if (arguments[I + 1])
				reinterpret_cast<TI*>(arguments[I + 1])->~TI();
		}

		template <class... Ts, std::size_t... Is>
		void DestroyArguments(RegisterState& state, const std::vector<std::uintptr_t>& arguments, const std::index_sequence<Is...>&)
		{
			(DestroyArgument<Ts, Is>(state, arguments), ...);
			state.m_RSP = arguments[0];
		}
	} // namespace MSAbi

	namespace SYSVAbi
	{
		template <class... Ts, std::size_t... Is>
		void PushArguments(RegisterState& state, std::vector<std::uintptr_t>& arguments, std::uintptr_t returnAddress, Ts&&..., const std::index_sequence<Is...>&)
		{
			arguments.resize(sizeof...(Ts) + 1);
			arguments[0] = state.m_RSP;
			if constexpr (sizeof...(Ts) > 0)
			{
			}
			state.m_RSP -= 8;
			state.pushq(returnAddress);
		}

		template <class... Ts, std::size_t... Is>
		void DestroyArguments(struct RegisterState& state, const std::vector<std::uintptr_t>& arguments, const std::index_sequence<Is...>&)
		{
			state.m_RSP = arguments[0];
		}
	} // namespace SYSVAbi

	template <class... Ts>
	void PushArguments(ECallingConvention callingConvention, RegisterState& state, std::vector<std::uintptr_t>& arguments, std::uintptr_t returnAddress, Ts&&... vs)
	{
		switch (callingConvention)
		{
		case ECallingConvention::MSAbi:
			MSAbi::PushArguments<Ts...>(state, arguments, returnAddress, std::forward<Ts>(vs)..., std::make_index_sequence<sizeof...(Ts)>());
			break;
		case ECallingConvention::SYSVAbi:
			SYSVAbi::PushArguments<Ts...>(state, arguments, returnAddress, std::forward<Ts>(vs)..., std::make_index_sequence<sizeof...(Ts)>());
			break;
		}
	}

	template <class... Ts>
	void DestroyArguments(ECallingConvention callingConvention, struct RegisterState& state, const std::vector<std::uintptr_t>& arguments)
	{
		switch (callingConvention)
		{
		case ECallingConvention::MSAbi:
			MSAbi::DestroyArguments<Ts...>(state, arguments, std::make_index_sequence<sizeof...(Ts)>());
			break;
		case ECallingConvention::SYSVAbi:
			SYSVAbi::DestroyArguments<Ts...>(state, arguments, std::make_index_sequence<sizeof...(Ts)>());
			break;
		}
	}
} // namespace Fibers