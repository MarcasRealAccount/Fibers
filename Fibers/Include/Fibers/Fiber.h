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

	template <class T, class... Args>
	concept Callable = requires(T&& t, Args&&... args)
	{
		t(std::forward<Args>(args)...);
	};

	class Fiber
	{
	public:
		Fiber(ECallingConvention callingConvention, std::size_t stackSize);
		template <class Function, class... Ts>
		requires Callable<Function, Ts...>
		Fiber(ECallingConvention callingConvention, std::size_t stackSize, Function&& function, Ts&&... vs);
		Fiber(Fiber&& move) noexcept;
		Fiber& operator=(Fiber&& move) noexcept;
		~Fiber();

		template <class Function, class... Ts>
		requires Callable<Function, Ts...>
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
		template <class... Ts>
		void destroyArguments();
		template <class Function>
		void destroyFunction();

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

		std::vector<std::uintptr_t> m_Arguments;

		std::uint64_t m_ID;

		void (*m_ArgumentDestructor)(Fiber&);
		void (*m_FunctionDestructor)(Fiber&);
	};
} // namespace Fibers

#include "CallingConventions.h"

namespace Fibers
{
	//----------------
	// Implementation
	//----------------

	template <class Function, class... Ts>
	requires Callable<Function, Ts...>
	Fiber::Fiber(ECallingConvention callingConvention, std::size_t stackSize, Function&& function, Ts&&... vs)
	    : Fiber(callingConvention, stackSize)
	{
		setFunction<Function, Ts...>(std::forward<Function>(function), std::forward<Ts>(vs)...);
	}

	template <class Function, class... Ts>
	requires Callable<Function, Ts...>
	void Fiber::setFunction(Function&& function, Ts&&... vs)
	{
		using ClassType = std::remove_reference_t<Function>;

		if constexpr (std::is_class_v<ClassType>)
		{
			m_State.push<Function>(std::forward<Function>(function));
			ClassType* newF      = reinterpret_cast<ClassType*>(m_State.m_RSP);
			m_FunctionDestructor = Utils::UBCast<decltype(m_FunctionDestructor)>(&Fiber::destroyFunction<Function>);
			PushArguments<ClassType*, Ts...>(m_CallingConvention, m_State, m_Arguments, reinterpret_cast<std::uintptr_t>(&ExitFiber), std::forward<ClassType*>(newF), std::forward<Ts>(vs)...);
			m_ArgumentDestructor = Utils::UBCast<decltype(m_ArgumentDestructor)>(&Fiber::destroyArguments<std::uintptr_t, Ts...>);
			m_State.m_RIP        = Utils::UBCast<std::uintptr_t>(&ClassType::operator());
		}
		else
		{
			m_FunctionDestructor = nullptr;
			PushArguments<Ts...>(m_CallingConvention, m_State, m_Arguments, reinterpret_cast<std::uintptr_t>(&ExitFiber), std::forward<Ts>(vs)...);
			m_ArgumentDestructor = Utils::UBCast<decltype(m_ArgumentDestructor)>(&Fiber::destroyArguments<Ts...>);
			m_State.m_RIP        = Utils::UBCast<std::uintptr_t>(function);
		}
		m_ArgumentStart = m_State.m_RSP;
	}

	template <class... Ts>
	void Fiber::destroyArguments()
	{
		DestroyArguments<Ts...>(m_CallingConvention, m_State, m_Arguments);
	}

	template <class Function>
	void Fiber::destroyFunction()
	{
		using ClassType = std::remove_reference_t<Function>;
		reinterpret_cast<ClassType*>(m_State.m_RSP)->~ClassType();
	}
} // namespace Fibers