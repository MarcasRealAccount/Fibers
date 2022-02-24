#pragma once

#include "Utils/Core.h"
#include "Utils/Utils.h"

#if BUILD_IS_SYSTEM_WINDOWS
#include "MSABI/Registers.h"
#else
#include "SYSV/Registers.h"
#endif

#include <cstddef>
#include <cstdint>

#include <type_traits>

namespace Fibers
{
	void yield();
	void exitFiber();

	class Fiber
	{
	public:
		Fiber();
		Fiber(Fiber&& move) noexcept;

		template <class Function, class... Args>
		Fiber(std::size_t stackSize, Function&& function, Args&&... args);

		~Fiber();

		void yield();
		void exit();
		void resume();

	private:
		void createStack(std::size_t stackSize);
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
		T pop();

#if BUILD_IS_SYSTEM_WINDOWS
		template <std::size_t Index, class Arg0, class... Args>
		void pushArgument(Arg0&& arg0, Args&&... args);
#else
		template <std::size_t IntIndex, std::size_t FloatIndex, class Arg0, class... Args>
		void pushArgument(Arg0&& arg0, Args&&... args);
#endif

		template <class... Args>
		void pushArguments(Args&&... args);

#if BUILD_IS_SYSTEM_WINDOWS
		template <std::size_t Index, class Arg0, class... Args>
		void destroyArgument();
#else
		template <std::size_t IntIndex, std::size_t FloatIndex, class Arg0, class... Args>
		void destroyArgument();
#endif

		template <class... Args>
		void destroyArguments();

		template <class Function, class... Args>
		void destroyEntry();

	private:
		std::uint8_t* m_Stack;
		std::size_t   m_StackSize;
		bool          m_Finished;
		bool          m_Entry;
		void (*m_Destructor)(Fiber& fiber);

		Details::EntrypointRegisters m_EntryState;

		Details::Registers m_State;
		Details::Registers m_ReturnState;
	};

	template <class Function, class... Args>
	Fiber::Fiber(std::size_t stackSize, Function&& function, Args&&... args)
	{
		m_Entry    = true;
		m_Finished = false;
		static_assert(std::is_invocable_v<Function, Args...>);
		using ClassType = std::remove_reference_t<Function>;

		createStack(stackSize);

		if constexpr (std::is_class_v<ClassType>)
		{
			push<Function>(std::forward<Function>(function));
			auto newF = reinterpret_cast<ClassType*>(m_State.m_RSP);
			pushArguments<ClassType*, Args...>(std::forward<ClassType*>(newF), std::forward<Args>(args)...);
			m_Destructor = Details::UBCast<decltype(m_Destructor)>(&Fiber::destroyEntry<Function, Args...>);
#if BUILD_IS_SYSTEM_WINDOWS
			m_State.m_RSP -= 0x20;
#endif
			pushq(reinterpret_cast<std::uintptr_t>(&exitFiber));
			m_State.m_RIP = Details::UBCast<std::uintptr_t>(&ClassType::operator());
		}
		else
		{
			pushArguments<Args...>(std::forward<Args>(args)...);
			m_Destructor = Details::UBCast<decltype(m_Destructor)>(&Fiber::destroyEntry<Function, Args...>);
#if BUILD_IS_SYSTEM_WINDOWS
			m_State.m_RSP -= 0x20;
#endif
			pushq(reinterpret_cast<std::uintptr_t>(&exitFiber));
			m_State.m_RIP = Details::UBCast<std::uintptr_t>(function);
		}
	}

	template <class T>
	std::size_t Fiber::push(T&& v)
	{
		std::size_t stackAllocationSize = (sizeof(T) + 15) / 16 * 16;
		m_State.m_RSP -= stackAllocationSize;
		*reinterpret_cast<T*>(m_State.m_RSP) = std::forward<T>(v);
		return stackAllocationSize;
	}

	template <class T>
	T Fiber::pop()
	{
		std::size_t stackAllocationSize = (sizeof(T) + 15) / 16 * 16;

		T v = std::forward<T>(*reinterpret_cast<T*>(m_State.m_RSP));
		m_State.m_RSP += stackAllocationSize;
		return v;
	}

#if BUILD_IS_SYSTEM_WINDOWS
	template <std::size_t Index, class Arg0, class... Args>
	void Fiber::pushArgument(Arg0&& arg0, Args&&... args)
	{
		if constexpr (sizeof...(Args) > 0)
			pushArgument<Index + 1, Args...>(std::forward<Args>(args)...);

		if constexpr (std::is_floating_point_v<Arg0>)
		{
			if constexpr (sizeof(Arg0) <= 16)
			{
				if constexpr (Index == 0)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM0) = std::forward<Arg0>(arg0);
				else if constexpr (Index == 1)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM1) = std::forward<Arg0>(arg0);
				else if constexpr (Index == 2)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM2) = std::forward<Arg0>(arg0);
				else if constexpr (Index == 3)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM3) = std::forward<Arg0>(arg0);
				else
					push<Arg0>(std::forward<Arg0>(arg0));
			}
			else
			{
				push<Arg0>(std::forward<Arg0>(arg0));
			}
		}
		else
		{
			if constexpr (sizeof(Arg0) <= 8)
			{
				if constexpr (Index == 0)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_RCX) = std::forward<Arg0>(arg0);
				else if constexpr (Index == 1)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_RDX) = std::forward<Arg0>(arg0);
				else if constexpr (Index == 2)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_R8) = std::forward<Arg0>(arg0);
				else if constexpr (Index == 3)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_R9) = std::forward<Arg0>(arg0);
				else
					push<Arg0>(std::forward<Arg0>(arg0));
			}
			else
			{
				push<Arg0>(std::forward<Arg0>(arg0));
			}
		}
	}
#else
	template <std::size_t IntIndex, std::size_t FloatIndex, class Arg0, class... Args>
	void Fiber::pushArgument(Arg0&& arg0, Args&&... args)
	{
		if constexpr (std::is_floating_point_v<Arg0>)
		{
			if constexpr (sizeof(Arg0) <= 16)
			{
				if constexpr (FloatIndex == 0)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM0) = std::forward<Arg0>(arg0);
				else if constexpr (FloatIndex == 1)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM1) = std::forward<Arg0>(arg0);
				else if constexpr (FloatIndex == 2)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM2) = std::forward<Arg0>(arg0);
				else if constexpr (FloatIndex == 3)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM3) = std::forward<Arg0>(arg0);
				else if constexpr (FloatIndex == 4)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM4) = std::forward<Arg0>(arg0);
				else if constexpr (FloatIndex == 5)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM5) = std::forward<Arg0>(arg0);
				else if constexpr (FloatIndex == 6)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM6) = std::forward<Arg0>(arg0);
				else if constexpr (FloatIndex == 7)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_XMM7) = std::forward<Arg0>(arg0);
				else
					push<Arg0>(std::forward<Arg0>(arg0));
			}
			else
			{
				push<Arg0>(std::forward<Arg0>(arg0));
			}

			if constexpr (sizeof...(Args) > 0)
				pushArgument<IntIndex, FloatIndex + 1, Args...>(std::forward<Args>(args)...);
		}
		else
		{
			if constexpr (sizeof(Arg0) <= 8)
			{
				if constexpr (IntIndex == 0)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_RDI) = std::forward<Arg0>(arg0);
				else if constexpr (IntIndex == 1)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_RSI) = std::forward<Arg0>(arg0);
				else if constexpr (IntIndex == 2)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_RDX) = std::forward<Arg0>(arg0);
				else if constexpr (IntIndex == 3)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_RCX) = std::forward<Arg0>(arg0);
				else if constexpr (IntIndex == 4)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_R8) = std::forward<Arg0>(arg0);
				else if constexpr (IntIndex == 5)
					*reinterpret_cast<Arg0*>(&m_EntryState.m_R9) = std::forward<Arg0>(arg0);
				else
					push<Arg0>(std::forward<Arg0>(arg0));
			}
			else
			{
				push<Arg0>(std::forward<Arg0>(arg0));
			}

			if constexpr (sizeof...(Args) > 0)
				pushArgument<IntIndex + 1, FloatIndex, Args...>(std::forward<Args>(args)...);
		}
	}
#endif

	template <class... Args>
	void Fiber::pushArguments(Args&&... args)
	{
		if constexpr (sizeof...(Args) > 0)
		{
#if BUILD_IS_SYSTEM_WINDOWS
			pushArgument<0, Args...>(std::forward<Args>(args)...);
#else
			pushArgument<0, 0, Args...>(std::forward<Args>(args)...);
#endif
		}
	}

#if BUILD_IS_SYSTEM_WINDOWS
	template <std::size_t Index, class Arg0, class... Args>
	void Fiber::destroyArgument()
	{
		if constexpr (sizeof...(Args) > 0)
			destroyArgument<Index + 1, Args...>();

		if constexpr (std::is_floating_point_v<Arg0>)
		{
			if constexpr (sizeof(Arg0) <= 16)
			{
				if constexpr (Index == 0)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM0)->~Arg0();
				else if constexpr (Index == 1)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM1)->~Arg0();
				else if constexpr (Index == 2)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM2)->~Arg0();
				else if constexpr (Index == 3)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM3)->~Arg0();
				else
				{
					reinterpret_cast<Arg0*>(m_State.m_RSP)->~Arg0();
					m_State.m_RSP += (sizeof(Arg0) + 15) / 16 * 16;
				}
			}
			else
			{
				reinterpret_cast<Arg0*>(m_State.m_RSP)->~Arg0();
				m_State.m_RSP += (sizeof(Arg0) + 15) / 16 * 16;
			}
		}
		else
		{
			if constexpr (sizeof(Arg0) <= 8)
			{
				if constexpr (Index == 0)
					reinterpret_cast<Arg0*>(&m_EntryState.m_RCX)->~Arg0();
				else if constexpr (Index == 1)
					reinterpret_cast<Arg0*>(&m_EntryState.m_RDX)->~Arg0();
				else if constexpr (Index == 2)
					reinterpret_cast<Arg0*>(&m_EntryState.m_R8)->~Arg0();
				else if constexpr (Index == 3)
					reinterpret_cast<Arg0*>(&m_EntryState.m_R9)->~Arg0();
				else
				{
					reinterpret_cast<Arg0*>(m_State.m_RSP)->~Arg0();
					m_State.m_RSP += (sizeof(Arg0) + 15) / 16 * 16;
				}
			}
			else
			{
				reinterpret_cast<Arg0*>(m_State.m_RSP)->~Arg0();
				m_State.m_RSP += (sizeof(Arg0) + 15) / 16 * 16;
			}
		}
	}
#else
	template <std::size_t IntIndex, std::size_t FloatIndex, class Arg0, class... Args>
	void Fiber::destroyArgument()
	{
		if constexpr (std::is_floating_point_v<Arg0>)
		{
			if constexpr (sizeof(Arg0) <= 16)
			{
				if constexpr (FloatIndex == 0)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM0)->~Arg0();
				else if constexpr (FloatIndex == 1)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM1)->~Arg0();
				else if constexpr (FloatIndex == 2)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM2)->~Arg0();
				else if constexpr (FloatIndex == 3)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM3)->~Arg0();
				else if constexpr (FloatIndex == 4)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM4)->~Arg0();
				else if constexpr (FloatIndex == 5)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM5)->~Arg0();
				else if constexpr (FloatIndex == 6)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM6)->~Arg0();
				else if constexpr (FloatIndex == 7)
					reinterpret_cast<Arg0*>(&m_EntryState.m_XMM7)->~Arg0();
				else
				{
					reinterpret_cast<Arg0*>(m_State.m_RSP)->~Arg0();
					m_State.m_RSP += (sizeof(Arg0) + 15) / 16 * 16;
				}
			}
			else
			{
				reinterpret_cast<Arg0*>(m_State.m_RSP)->~Arg0();
				m_State.m_RSP += (sizeof(Arg0) + 15) / 16 * 16;
			}

			if constexpr (sizeof...(Args) > 0)
				pushArgument<IntIndex, FloatIndex + 1, Args...>();
		}
		else
		{
			if constexpr (sizeof(Arg0) <= 8)
			{
				if constexpr (IntIndex == 0)
					reinterpret_cast<Arg0*>(&m_EntryState.m_RDI)->~Arg0();
				else if constexpr (IntIndex == 1)
					reinterpret_cast<Arg0*>(&m_EntryState.m_RSI)->~Arg0();
				else if constexpr (IntIndex == 2)
					reinterpret_cast<Arg0*>(&m_EntryState.m_RDX)->~Arg0();
				else if constexpr (IntIndex == 3)
					reinterpret_cast<Arg0*>(&m_EntryState.m_RCX)->~Arg0();
				else if constexpr (IntIndex == 4)
					reinterpret_cast<Arg0*>(&m_EntryState.m_R8)->~Arg0();
				else if constexpr (IntIndex == 5)
					reinterpret_cast<Arg0*>(&m_EntryState.m_R9)->~Arg0();
				else
				{
					reinterpret_cast<Arg0*>(m_State.m_RSP)->~Arg0();
					m_State.m_RSP += (sizeof(Arg0) + 15) / 16 * 16;
				}
			}
			else
			{
				reinterpret_cast<Arg0*>(m_State.m_RSP)->~Arg0();
				m_State.m_RSP += (sizeof(Arg0) + 15) / 16 * 16;
			}

			if constexpr (sizeof...(Args) > 0)
				pushArgument<IntIndex + 1, FloatIndex, Args...>();
		}
	}
#endif

	template <class... Args>
	void Fiber::destroyArguments()
	{
		if constexpr (sizeof...(Args) > 0)
		{
#if BUILD_IS_SYSTEM_WINDOWS
			destroyArgument<0, Args...>();
#else
			destroyArgument<0, 0, Args...>();
#endif
		}
	}

	template <class Function, class... Args>
	void Fiber::destroyEntry()
	{
		using ClassType = std::remove_reference_t<Function>;

		destroyArguments<Args...>();
		if constexpr (std::is_class_v<ClassType>)
			reinterpret_cast<ClassType*>(m_State.m_RSP)->~Function();
	}
} // namespace Fibers