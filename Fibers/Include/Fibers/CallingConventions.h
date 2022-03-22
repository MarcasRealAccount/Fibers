#pragma once

#include "CallingConvention.h"
#include "Stack.h"
#include "State.h"

#include <cstring>

#include <vector>

namespace Fibers
{
	namespace Utils
	{
		template <class T>
		struct ConstType
		{
			using Type = T;
		};

		template <std::size_t S>
		struct SizedInt;

		template <>
		struct SizedInt<1> : public ConstType<std::uint8_t>
		{
		};

		template <>
		struct SizedInt<2> : public ConstType<std::uint16_t>
		{
		};

		template <>
		struct SizedInt<4> : public ConstType<std::uint32_t>
		{
		};

		template <>
		struct SizedInt<8> : public ConstType<std::uint64_t>
		{
		};

		template <std::size_t S>
		using SizedIntT = typename SizedInt<S>::Type;

		template <class T>
		struct ToNative : public ConstType<T>
		{
		};

		template <class T>
		struct ToNative<T&> : public ConstType<std::uintptr_t>
		{
		};

		template <class T>
		struct ToNative<const T&> : public ConstType<std::uintptr_t>
		{
		};

		template <class T>
		struct ToNative<volatile T&> : public ConstType<std::uintptr_t>
		{
		};

		template <class T>
		struct ToNative<const volatile T&> : public ConstType<std::uintptr_t>
		{
		};

		template <class T>
		using ToNativeT = typename ToNative<T>::Type;

		template <class T>
		[[nodiscard]] constexpr std::conditional_t<std::is_reference_v<T>, ToNativeT<T>, ToNativeT<T>&&> ToNativeVal(std::remove_reference_t<T>& original) noexcept
		{
			if constexpr (std::is_reference_v<T>)
				return reinterpret_cast<ToNativeT<T>>(&original);
			else
				return static_cast<ToNativeT<T>&&>(original);
		}

		template <class T>
		[[nodiscard]] constexpr std::conditional_t<std::is_reference_v<T>, ToNativeT<T>, ToNativeT<T>&&> ToNativeVal(std::remove_reference_t<T>&& original) noexcept
		{
			if constexpr (std::is_reference_v<T>)
				return reinterpret_cast<ToNativeT<T>>(&original);
			else
				return static_cast<ToNativeT<T>&&>(original);
		}
	} // namespace Utils

	template <class T>
	std::size_t RequiredSize(ECallingConvention callingConvention);

	template <class T>
	std::size_t RequiredAlignedSize(ECallingConvention callingConvention, std::uintptr_t& rsp, std::uint64_t minAlignment = 0);

	namespace MSAbi
	{
		template <class T, std::size_t I>
		void RequiredStackAddressSize(std::size_t& rsp)
		{
			if constexpr (sizeof(T) > 8)
				RequiredAlignedSize<T>(ECallingConvention::MSAbi, rsp, 16);
		}

		template <class T, std::size_t I>
		void RequiredStackSize(std::size_t& rsp)
		{
			if constexpr (sizeof(T) > 8)
				RequiredAlignedSize<std::uintptr_t>(ECallingConvention::SYSVAbi, rsp);
			else if constexpr (I >= 4)
				RequiredAlignedSize<T>(ECallingConvention::MSAbi, rsp);
		}

		template <class T, std::size_t I>
		void AddStackAddress(std::vector<std::uintptr_t>& arguments, RegisterState& state, std::remove_reference_t<T>&& v)
		{
			if constexpr (sizeof(T) > 8)
			{
				state.push<T>(std::forward<T>(v), 16);
				arguments[I + 1] = state.m_RSP;
			}
		}

		template <std::size_t I, class T, class... Ts>
		void PushArgument(std::vector<std::uintptr_t>& arguments, RegisterState& state, std::remove_reference_t<T>&& v, std::remove_reference_t<Ts>&&... vs)
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
				if constexpr (I < 4)
				{
					if constexpr (std::is_floating_point_v<T>)
					{
						if constexpr (I == 0)
							*reinterpret_cast<T*>(&state.m_XMM0) = std::forward<T>(v);
						else if constexpr (I == 1)
							*reinterpret_cast<T*>(&state.m_XMM1) = std::forward<T>(v);
						else if constexpr (I == 2)
							*reinterpret_cast<T*>(&state.m_XMM2) = std::forward<T>(v);
						else if constexpr (I == 3)
							*reinterpret_cast<T*>(&state.m_XMM3) = std::forward<T>(v);
					}
					else
					{
						if constexpr (I == 0)
							*reinterpret_cast<T*>(&state.m_RCX) = std::forward<T>(v);
						else if constexpr (I == 1)
							*reinterpret_cast<T*>(&state.m_RDX) = std::forward<T>(v);
						else if constexpr (I == 2)
							*reinterpret_cast<T*>(&state.m_R8) = std::forward<T>(v);
						else if constexpr (I == 3)
							*reinterpret_cast<T*>(&state.m_R9) = std::forward<T>(v);
					}
				}
				else
				{
					state.push<T>(std::forward<T>(v));
					arguments[I + 1] = state.m_RSP;
				}
			}
		}

		template <class... Ts, std::size_t... Is>
		void PushArguments(RegisterState& state, std::vector<std::uintptr_t>& arguments, std::uintptr_t returnAddress, std::remove_reference_t<Ts>&&... vs, const std::index_sequence<Is...>&)
		{
			arguments.resize(sizeof...(Ts) + 1);
			arguments[0] = state.m_RSP;

			std::size_t rsp = state.m_RSP;
			(RequiredStackAddressSize<Ts, Is>(rsp), ...);
			(RequiredStackSize<Ts, Is>(rsp), ...);
			std::size_t size = state.m_RSP - rsp;
			rsp &= ~15ULL;
			std::size_t alignedSize = state.m_RSP - rsp;

			if constexpr (sizeof...(Ts) > 0)
			{
				(AddStackAddress<Ts, Is>(arguments, state, std::forward<Ts>(vs)), ...);
				state.m_RSP -= alignedSize - size;

				PushArgument<0, Ts...>(arguments, state, std::forward<Ts>(vs)...);
			}
			state.m_RSP -= 32;
			state.pushq(returnAddress);
		}

		template <class... Ts, std::size_t... Is>
		void DestroyArguments(RegisterState& state, const std::vector<std::uintptr_t>& arguments, const std::index_sequence<Is...>&)
		{
			state.m_RSP = arguments[0];
		}
	} // namespace MSAbi

	namespace SYSVAbi
	{
		template <std::size_t Integer, std::size_t SSE, class T>
		constexpr std::size_t GetSSECount()
		{
			if constexpr (SSE < 8 && std::is_floating_point_v<T>)
				return 1;
			else
				return 0;
		}

		template <std::size_t Integer, std::size_t SSE, class T>
		constexpr std::size_t GetIntegerCount()
		{
			if constexpr (GetSSECount<Integer, SSE, T>() == 0)
			{
				if constexpr (Integer < 6 && (std::is_integral_v<T> || std::is_pointer_v<T> || !std::is_trivially_copyable_v<T> || !std::is_trivially_destructible_v<T>) )
					return 1;
				else if constexpr (sizeof(T) < 16 && (sizeof(T) + 7) / 8 < 6 - Integer)
					return (sizeof(T) + 7) / 8;
				else
					return 0;
			}
			else
			{
				return 0;
			}
		}

		template <std::size_t Integer, std::size_t SSE, class T>
		static constexpr std::size_t SSECount = GetSSECount<Integer, SSE, T>();
		template <std::size_t Integer, std::size_t SSE, class T>
		static constexpr std::size_t IntegerCount = GetIntegerCount<Integer, SSE, T>();

		template <class T>
		void RequiredStackAddressSize(std::size_t& rsp)
		{
			if constexpr (!std::is_trivially_copyable_v<T> || !std::is_trivially_destructible_v<T>)
				RequiredAlignedSize<T>(ECallingConvention::SYSVAbi, rsp, 16);
		}

		template <std::size_t Integer, std::size_t SSE, class T, class... Ts>
		void RequiredStackSize(std::size_t& rsp)
		{
			if constexpr (sizeof...(Ts) > 0)
				RequiredStackSize<Integer + IntegerCount<Integer, SSE, T>, SSE + SSECount<Integer, SSE, T>, Ts...>(rsp);

			if constexpr (!std::is_trivially_copyable_v<T> || !std::is_trivially_destructible_v<T>)
			{
				if constexpr (Integer >= 6)
					RequiredAlignedSize<std::uintptr_t>(ECallingConvention::SYSVAbi, rsp);
			}
			else if constexpr (sizeof(T) > 16 || (sizeof(T) < 16 && sizeof(T) / 8 > 6 - Integer))
			{
				RequiredAlignedSize<T>(ECallingConvention::SYSVAbi, rsp);
			}
			else if constexpr (Integer >= 6 || SSE >= 8)
			{
				RequiredAlignedSize<T>(ECallingConvention::SYSVAbi, rsp);
			}
		}

		template <class T, std::size_t I>
		void AddStackAddress(std::vector<std::uintptr_t>& arguments, RegisterState& state, std::remove_reference_t<T>&& v)
		{
			if constexpr (!std::is_trivially_copyable_v<T> || !std::is_trivially_destructible_v<T>)
			{
				state.push<T>(std::forward<T>(v), 16);
				arguments[I + 1] = state.m_RSP;
			}
		}

		template <std::size_t Integer, std::size_t SSE, std::size_t I, class T, class... Ts>
		void PushArgument(RegisterState& state, std::vector<std::uintptr_t>& arguments, std::remove_reference_t<T>&& v, std::remove_reference_t<Ts>&&... vs)
		{
			if constexpr (sizeof...(Ts) > 0)
				PushArgument<Integer + IntegerCount<Integer, SSE, T>, SSE + SSECount<Integer, SSE, T>, I + 1, Ts...>(state, arguments, std::forward<Ts>(vs)...);

			if (arguments[I + 1])
			{
				if constexpr (Integer == 0)
					*reinterpret_cast<std::uintptr_t*>(&state.m_RDI) = arguments[I + 1];
				else if constexpr (Integer == 1)
					*reinterpret_cast<std::uintptr_t*>(&state.m_RSI) = arguments[I + 1];
				else if constexpr (Integer == 2)
					*reinterpret_cast<std::uintptr_t*>(&state.m_RDX) = arguments[I + 1];
				else if constexpr (Integer == 3)
					*reinterpret_cast<std::uintptr_t*>(&state.m_RCX) = arguments[I + 1];
				else if constexpr (Integer == 4)
					*reinterpret_cast<std::uintptr_t*>(&state.m_R8) = arguments[I + 1];
				else if constexpr (Integer == 5)
					*reinterpret_cast<std::uintptr_t*>(&state.m_R9) = arguments[I + 1];
				else
					state.pushq(arguments[I + 1]);
			}
			else
			{
				if constexpr (sizeof(T) > 16 || (sizeof(T) < 16 && sizeof(T) / 8 > 6 - Integer))
				{
					state.push<T>(std::forward<T>(v));
					arguments[I + 1] = state.m_RSP;
				}
				else if constexpr ((std::is_integral_v<T> || std::is_pointer_v<T>) &&Integer < 6)
				{
					if constexpr (Integer == 0)
						*reinterpret_cast<T*>(&state.m_RDI) = std::forward<T>(v);
					else if constexpr (Integer == 1)
						*reinterpret_cast<T*>(&state.m_RSI) = std::forward<T>(v);
					else if constexpr (Integer == 2)
						*reinterpret_cast<T*>(&state.m_RDX) = std::forward<T>(v);
					else if constexpr (Integer == 3)
						*reinterpret_cast<T*>(&state.m_RCX) = std::forward<T>(v);
					else if constexpr (Integer == 4)
						*reinterpret_cast<T*>(&state.m_R8) = std::forward<T>(v);
					else if constexpr (Integer == 5)
						*reinterpret_cast<T*>(&state.m_R9) = std::forward<T>(v);
				}
				else if constexpr (std::is_floating_point_v<T> && SSE < 8)
				{
					if constexpr (SSE == 0)
						*reinterpret_cast<T*>(&state.m_XMM0) = std::forward<T>(v);
					else if constexpr (SSE == 1)
						*reinterpret_cast<T*>(&state.m_XMM1) = std::forward<T>(v);
					else if constexpr (SSE == 2)
						*reinterpret_cast<T*>(&state.m_XMM2) = std::forward<T>(v);
					else if constexpr (SSE == 3)
						*reinterpret_cast<T*>(&state.m_XMM3) = std::forward<T>(v);
					else if constexpr (SSE == 4)
						*reinterpret_cast<T*>(&state.m_XMM4) = std::forward<T>(v);
					else if constexpr (SSE == 5)
						*reinterpret_cast<T*>(&state.m_XMM5) = std::forward<T>(v);
					else if constexpr (SSE == 6)
						*reinterpret_cast<T*>(&state.m_XMM6) = std::forward<T>(v);
					else if constexpr (SSE == 7)
						*reinterpret_cast<T*>(&state.m_XMM7) = std::forward<T>(v);
				}
				else if constexpr (sizeof(T) < 16 && (sizeof(T) + 7) / 8 < 6 - Integer)
				{
					constexpr std::size_t Regs = (sizeof(T) + 7) / 8;

					T* temp = reinterpret_cast<T*>(calloc(1, Regs * 8));
					*temp   = std::forward<T>(v);

					std::size_t reg = Integer;
					for (std::size_t i = 0; i < Regs; ++i, ++reg)
					{
						std::uint64_t value;
						if (i < Regs - 1)
							value = reinterpret_cast<std::uint64_t*>(temp)[i];
						else
							value = *reinterpret_cast<Utils::SizedIntT<sizeof(T) - Regs - 1>*>(reinterpret_cast<std::uint8_t*>(temp) + i * 8);

						switch (reg)
						{
						case 0:
							*reinterpret_cast<T*>(&state.m_RDI) = value;
							break;
						case 1:
							*reinterpret_cast<T*>(&state.m_RSI) = value;
							break;
						case 2:
							*reinterpret_cast<T*>(&state.m_RDX) = value;
							break;
						case 3:
							*reinterpret_cast<T*>(&state.m_RCX) = value;
							break;
						case 4:
							*reinterpret_cast<T*>(&state.m_R8) = value;
							break;
						case 5:
							*reinterpret_cast<T*>(&state.m_R9) = value;
							break;
						}
					}

					free(temp);
				}
				else
				{
					state.push<T>(std::forward<T>(v));
					arguments[I + 1] = state.m_RSP;
				}
			}
		}

		template <class... Ts, std::size_t... Is>
		void PushArguments(RegisterState& state, std::vector<std::uintptr_t>& arguments, std::uintptr_t returnAddress, std::remove_reference_t<Ts>&&... vs, const std::index_sequence<Is...>&)
		{
			arguments.resize(sizeof...(Ts) + 1);
			arguments[0] = state.m_RSP;

			if constexpr (sizeof...(Ts) > 0)
			{
				(AddStackAddress<Ts, Is>(arguments, state, std::forward<Ts>(vs)), ...);
				state.m_RSP &= ~15ULL;

				std::size_t rsp = state.m_RSP;
				RequiredStackSize<0, 0, Ts...>(rsp);
				std::size_t size = state.m_RSP - rsp;
				rsp &= ~15ULL;
				std::size_t alignedSize = state.m_RSP - rsp;
				state.m_RSP -= alignedSize - size;

				PushArgument<0, 0, 0, Ts...>(state, arguments, std::forward<Ts>(vs)...);
			}
			state.pushq(returnAddress);
		}

		template <std::size_t Integer, std::size_t SSE, std::size_t I, class T, class... Ts>
		void DestroyArgument(RegisterState& state, const std::vector<std::uintptr_t>& arguments)
		{
			if constexpr (sizeof...(Ts) > 0)
				DestroyArgument<Integer + IntegerCount<Integer, SSE, T>, SSE + SSECount<Integer, SSE, T>, I + 1, Ts...>(state, arguments);

			if (arguments[I + 1])
			{
				reinterpret_cast<T*>(arguments[I + 1])->~T();
			}
			else
			{
				if constexpr (std::is_integral_v<T> || std::is_pointer_v<T>)
				{
					if constexpr (Integer == 0)
						reinterpret_cast<T*>(&state.m_RDI)->~T();
					else if constexpr (Integer == 1)
						reinterpret_cast<T*>(&state.m_RSI)->~T();
					else if constexpr (Integer == 2)
						reinterpret_cast<T*>(&state.m_RDX)->~T();
					else if constexpr (Integer == 3)
						reinterpret_cast<T*>(&state.m_RCX)->~T();
					else if constexpr (Integer == 4)
						reinterpret_cast<T*>(&state.m_R8)->~T();
					else if constexpr (Integer == 5)
						reinterpret_cast<T*>(&state.m_R9)->~T();
				}
				else if constexpr (std::is_floating_point_v<T>)
				{
					if constexpr (SSE == 0)
						reinterpret_cast<T*>(&state.m_XMM0)->~T();
					else if constexpr (SSE == 1)
						reinterpret_cast<T*>(&state.m_XMM1)->~T();
					else if constexpr (SSE == 2)
						reinterpret_cast<T*>(&state.m_XMM2)->~T();
					else if constexpr (SSE == 3)
						reinterpret_cast<T*>(&state.m_XMM3)->~T();
					else if constexpr (SSE == 4)
						reinterpret_cast<T*>(&state.m_XMM4)->~T();
					else if constexpr (SSE == 5)
						reinterpret_cast<T*>(&state.m_XMM5)->~T();
					else if constexpr (SSE == 6)
						reinterpret_cast<T*>(&state.m_XMM6)->~T();
					else if constexpr (SSE == 7)
						reinterpret_cast<T*>(&state.m_XMM7)->~T();
				}
				else if constexpr (sizeof(T) < 16)
				{
					constexpr std::size_t Regs = (sizeof(T) + 7) / 8;

					T* temp = reinterpret_cast<T*>(calloc(1, Regs * 8));

					std::size_t reg = Integer;
					for (std::size_t i = 0; i < Regs; ++i, ++reg)
					{
						std::uint64_t value;

						switch (reg)
						{
						case 0:
							value = state.m_RDI;
							break;
						case 1:
							value = state.m_RSI;
							break;
						case 2:
							value = state.m_RDX;
							break;
						case 3:
							value = state.m_RCX;
							break;
						case 4:
							value = state.m_R8;
							break;
						case 5:
							value = state.m_R9;
							break;
						}

						reinterpret_cast<std::uint64_t*>(temp)[i] = value;
					}

					temp->~T();
					free(temp);
				}
			}
		}

		template <class... Ts, std::size_t... Is>
		void DestroyArguments(RegisterState& state, const std::vector<std::uintptr_t>& arguments, const std::index_sequence<Is...>&)
		{
			DestroyArgument<0, 0, 0, Ts...>(state, arguments);
			state.m_RSP = arguments[0];
		}
	} // namespace SYSVAbi

	template <class T>
	std::size_t RequiredSize(ECallingConvention callingConvention)
	{
		switch (callingConvention)
		{
		case ECallingConvention::Native: [[fallthrough]];
		case ECallingConvention::MSAbi: return (sizeof(T) + 7) / 8 * 8;
		case ECallingConvention::SYSVAbi: return (sizeof(T) + 7) / 8 * 8;
		}
		return sizeof(T);
	}

	template <class T>
	std::size_t RequiredAlignedSize(ECallingConvention callingConvention, std::uintptr_t& rsp, std::uint64_t minAlignment)
	{
		std::size_t allocationSize = RequiredSize<T>(callingConvention);
		std::size_t alignment      = std::max(minAlignment, static_cast<std::uint64_t>(alignof(T)));
		std::size_t diff           = rsp;
		rsp                        = (rsp - allocationSize) / alignment * alignment;
		diff -= rsp;
		return allocationSize + diff;
	}

	template <class... Ts>
	void PushArguments(ECallingConvention callingConvention, RegisterState& state, std::vector<std::uintptr_t>& arguments, std::uintptr_t returnAddress, Ts&&... vs)
	{
		switch (callingConvention)
		{
		case ECallingConvention::Native: [[fallthrough]];
		case ECallingConvention::MSAbi:
			MSAbi::PushArguments<Utils::ToNativeT<Ts>...>(state, arguments, returnAddress, Utils::ToNativeVal<Ts>(vs)..., std::make_index_sequence<sizeof...(Ts)>());
			break;
		case ECallingConvention::SYSVAbi:
			SYSVAbi::PushArguments<Utils::ToNativeT<Ts>...>(state, arguments, returnAddress, Utils::ToNativeVal<Ts>(vs)..., std::make_index_sequence<sizeof...(Ts)>());
			break;
		}
	}

	template <class... Ts>
	void DestroyArguments(ECallingConvention callingConvention, RegisterState& state, const std::vector<std::uintptr_t>& arguments)
	{
		switch (callingConvention)
		{
		case ECallingConvention::Native: [[fallthrough]];
		case ECallingConvention::MSAbi:
			MSAbi::DestroyArguments<Utils::ToNativeT<Ts>...>(state, arguments, std::make_index_sequence<sizeof...(Ts)>());
			break;
		case ECallingConvention::SYSVAbi:
			SYSVAbi::DestroyArguments<Utils::ToNativeT<Ts>...>(state, arguments, std::make_index_sequence<sizeof...(Ts)>());
			break;
		}
	}
} // namespace Fibers