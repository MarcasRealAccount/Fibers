#pragma once

#include "CallingConvention.h"
#include "Stack.h"
#include "State.h"
#include "Utils/Utils.h"

#include <vector>

namespace Fibers
{
	class Fiber* GetCurrentFiber();
	class Fiber* GetFiber(std::uint64_t id);

	void Yield();
	void ExitFiber();

	template <class T>
	concept Callable = std::is_invocable_v<T>;

	class Fiber
	{
	public:
		Fiber(ECallingConvention callingConvention, std::size_t stackSize);
		template <Callable Function, class... Ts>
		Fiber(ECallingConvention callingConvention, std::size_t stackSize, Function&& function, Ts&&... vs);
		Fiber(Fiber&& move) noexcept;
		Fiber& operator=(Fiber&& move) noexcept;
		~Fiber();

		template <Callable Function, class... Ts>
		void setFunction(Function&& function, Ts&&... vs);

		void resume();
		void exit();
		void yield();

		void addFiberLocal(struct FiberLocalBase* fiberLocal);
		void removeFiberLocal(struct FiberLocalBase* fiberLocal);

		auto  getCallingConvention() const { return m_CallingConvention; }
		auto& getStack() const { return m_Stack; }
		auto  isFinished() const { return m_Finished; }
		auto& getFiberLocals() const { return m_FiberLocals; }
		auto& getState() const { return m_State; }
		auto& getReturnState() const { return m_ReturnState; }
		auto  getID() const { return m_ID; }

		explicit operator bool() const { return !m_Finished; }

	private:
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

		template <class... Ts>
		void pushArguments(std::uintptr_t returnAddress, Ts&&... vs);

		template <class Function>
		void destroyFunction();
		template <class... Ts>
		void destroyArguments();

	private:
		ECallingConvention m_CallingConvention;

		Stack m_Stack;
		bool  m_Entry;
		bool  m_Finished;
		bool  m_Inside;

		std::vector<struct FiberLocalBase*> m_FiberLocals;

		RegisterState m_State;
		RegisterState m_ReturnState;
		std::uint64_t m_ArgumentStart;

		std::uint64_t m_ID;

		void (*m_FunctionDestructor)(Fiber&);
		void (*m_ArgumentDestructor)(Fiber&);
	};
} // namespace Fibers

#include "CallingConventions.h"

namespace Fibers
{
	//----------------
	// Implementation
	//----------------

	template <Callable Function, class... Ts>
	Fiber::Fiber(ECallingConvention callingConvention, std::size_t stackSize, Function&& function, Ts&&... vs)
	    : Fiber(callingConvention, stackSize)
	{
		setFunction<Function, Ts...>(std::forward<Function>(function), std::forward<Ts>(vs)...);
	}

	template <Callable Function, class... Ts>
	void Fiber::setFunction(Function&& function, Ts&&... vs)
	{
		using ClassType = std::remove_reference_t<Function>;

		if constexpr (std::is_class_v<ClassType>)
		{
			push<Function>(std::forward<Function>(function));
			m_FunctionDestructor = Utils::UBCast<decltype(m_FunctionDestructor)>(&Fiber::destroyFunction<Function>);
			m_State.m_RIP        = Utils::UBCast<std::uintptr_t>(&ClassType::operator());
		}
		else
		{
			m_FunctionDestructor = nullptr;
			m_State.m_RIP        = Utils::UBCast<std::uintptr_t>(function);
		}
		pushArguments<Ts...>(reinterpret_cast<std::uintptr_t>(&ExitFiber), std::forward<Ts>(vs)...);
		m_ArgumentStart      = m_State.m_RSP;
		m_ArgumentDestructor = Utils::UBCast<decltype(m_ArgumentDestructor)>(&Fiber::destroyArguments<Ts...>);
	}

	template <class T>
	std::size_t Fiber::push(T&& v)
	{
		std::size_t allocationSize = 0;
		switch (m_CallingConvention)
		{
		case ECallingConvention::MSAbi:
			allocationSize = (sizeof(T) + 7) / 8 * 8;
			break;
		case ECallingConvention::SYSVAbi:
			allocationSize = (sizeof(T) + 15) / 16 * 16;
			break;
		}
		m_State.m_RSP -= allocationSize;
		*reinterpret_cast<T*>(m_State.m_RSP) = std::forward<T>(v);
		return allocationSize;
	}

	template <class T>
	T Fiber::pop()
	{
		std::size_t allocationSize = 0;
		switch (m_CallingConvention)
		{
		case ECallingConvention::MSAbi:
			allocationSize = (sizeof(T) + 7) / 8 * 8;
			break;
		case ECallingConvention::SYSVAbi:
			allocationSize = (sizeof(T) + 15) / 16 * 16;
			break;
		}
		T v = std::forward<T>(*reinterpret_cast<T*>(m_State.m_RSP));
		m_State.m_RSP += allocationSize;
		return v;
	}

	template <class... Ts>
	void Fiber::pushArguments(std::uintptr_t returnAddress, Ts&&... vs)
	{
		PushArguments<Ts...>(m_CallingConvention, m_State, returnAddress, std::forward<Ts>(vs)...);
	}

	template <class Function>
	void Fiber::destroyFunction()
	{
		using ClassType = std::remove_reference_t<Function>;
		reinterpret_cast<ClassType*>(m_State.m_RSP)->~ClassType();
	}

	template <class... Ts>
	void Fiber::destroyArguments()
	{
		DestroyArguments<Ts...>(m_CallingConvention, m_State);
	}
} // namespace Fibers