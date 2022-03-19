#include "Fibers/Fiber.h"
#include "Fibers/FiberLocal.h"
#include "Fibers/Utils/Core.h"

#include <random>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace Fibers
{
	static std::unordered_map<std::uint64_t, Fiber*> s_Fibers;

	static std::shared_mutex s_FibersMutex;

	static std::mt19937_64 s_FiberRNG { std::random_device {}() };

	thread_local std::vector<Fiber*> s_FiberStack;

	static std::uint64_t NewFiberID()
	{
		std::shared_lock lock(s_FibersMutex);

		auto id = s_FiberRNG();
		while (id == 0ULL || GetFiber(id))
			id = s_FiberRNG();
		return id;
	}

	static void AddFiber(Fiber* fiber)
	{
		std::unique_lock lock(s_FibersMutex);

		s_Fibers.insert({ fiber->getID(), fiber });
	}

	static void RemoveFiber(Fiber* fiber)
	{
		std::unique_lock lock(s_FibersMutex);

		auto itr = s_Fibers.find(fiber->getID());
		if (itr != s_Fibers.end())
			s_Fibers.erase(itr);
	}

	Fiber* GetCurrentFiber()
	{
		return s_FiberStack.size() > 0 ? s_FiberStack.back() : nullptr;
	}

	Fiber* GetFiber(std::uint64_t id)
	{
		std::shared_lock lock(s_FibersMutex);

		auto itr = s_Fibers.find(id);
		return itr != s_Fibers.end() ? itr->second : nullptr;
	}

	void Yield()
	{
		auto fiber = GetCurrentFiber();
		if (fiber)
			fiber->yield();
	}

	void ExitFiber()
	{
		GetCurrentFiber()->exit();
	}

	Fiber::Fiber(ECallingConvention callingConvention, std::size_t stackSize)
	    : m_CallingConvention(CorrectCallingConvention(callingConvention)), m_Stack(m_CallingConvention, stackSize), m_Entry(true), m_Finished(false), m_Inside(false), m_State(m_CallingConvention), m_ReturnState(ECallingConvention::Native), m_ID(NewFiberID()), m_FunctionDestructor(nullptr), m_ArgumentDestructor(nullptr)
	{
		AddFiber(this);
		auto stackTop = reinterpret_cast<std::uintptr_t>(m_Stack.getTop());
		m_State.m_RBP = stackTop;
		m_State.m_RSP = stackTop;
	}

	Fiber::Fiber(Fiber&& move) noexcept
	    : m_CallingConvention(move.m_CallingConvention), m_Stack(std::move(move.m_Stack)), m_Entry(move.m_Entry), m_Finished(move.m_Finished), m_Inside(move.m_Inside), m_FiberLocals(std::move(move.m_FiberLocals)), m_State(move.m_State), m_ReturnState(move.m_ReturnState), m_ID(move.m_ID), m_FunctionDestructor(move.m_FunctionDestructor), m_ArgumentDestructor(move.m_ArgumentDestructor)
	{
		RemoveFiber(&move);
		AddFiber(this);
		move.m_State              = { ECallingConvention::Native };
		move.m_ReturnState        = { ECallingConvention::Native };
		move.m_ID                 = 0ULL;
		move.m_FunctionDestructor = nullptr;
		move.m_ArgumentDestructor = nullptr;
	}

	Fiber& Fiber::operator=(Fiber&& move) noexcept
	{
		m_CallingConvention  = move.m_CallingConvention;
		m_Stack              = std::move(move.m_Stack);
		m_Entry              = move.m_Entry;
		m_Finished           = move.m_Finished;
		m_Inside             = move.m_Inside;
		m_FiberLocals        = std::move(move.m_FiberLocals);
		m_State              = move.m_State;
		m_ReturnState        = move.m_ReturnState;
		m_ID                 = move.m_ID;
		m_FunctionDestructor = move.m_FunctionDestructor;
		m_ArgumentDestructor = move.m_ArgumentDestructor;
		RemoveFiber(&move);
		AddFiber(this);
		move.m_State              = { ECallingConvention::Native };
		move.m_ReturnState        = { ECallingConvention::Native };
		move.m_ID                 = 0ULL;
		move.m_FunctionDestructor = nullptr;
		move.m_ArgumentDestructor = nullptr;
		return *this;
	}

	Fiber::~Fiber()
	{
		exit();

		for (auto local : m_FiberLocals)
			local->removeFiber(m_ID);
		m_FiberLocals.clear();

		RemoveFiber(this);
	}

	void Fiber::resume()
	{
		if (m_Finished)
			return;

		s_FiberStack.push_back(this);
		auto entry = m_Entry;
		if (m_Entry)
			m_Entry = false;
		m_Inside = true;
		storeAndRestore(m_ReturnState, nullptr, m_State, entry);
	}

	void Fiber::exit()
	{
		if (m_Finished)
			return;

		if (m_Inside)
		{
			s_FiberStack.pop_back();
			m_Inside   = false;
			m_Finished = true;
			m_State.store();
		}
		m_State.m_RSP = m_ArgumentStart;
		if (m_ArgumentDestructor)
			m_ArgumentDestructor(*this);
		if (m_FunctionDestructor)
			m_FunctionDestructor(*this);
		m_ReturnState.restore();
	}

	void Fiber::yield()
	{
		if (m_Finished)
			return;

		s_FiberStack.pop_back();
		m_Inside = false;
		storeAndRestore(m_State, nullptr, m_ReturnState, false);
	}

	void Fiber::addFiberLocal(struct FiberLocalBase* fiberLocal)
	{
		m_FiberLocals.push_back(fiberLocal);
	}

	void Fiber::removeFiberLocal(struct FiberLocalBase* fiberLocal)
	{
		auto itr = std::find(m_FiberLocals.begin(), m_FiberLocals.end(), fiberLocal);
		if (itr != m_FiberLocals.end())
			m_FiberLocals.erase(itr);
	}

	void Fiber::pushb(std::uint8_t byte)
	{
		m_State.m_RSP -= 1;
		*reinterpret_cast<std::uint8_t*>(m_State.m_RSP) = byte;
	}

	void Fiber::pushw(std::uint16_t word)
	{
		m_State.m_RSP -= 2;
		*reinterpret_cast<std::uint16_t*>(m_State.m_RSP) = word;
	}

	void Fiber::pushd(std::uint32_t dword)
	{
		m_State.m_RSP -= 4;
		*reinterpret_cast<std::uint32_t*>(m_State.m_RSP) = dword;
	}

	void Fiber::pushq(std::uint64_t qword)
	{
		m_State.m_RSP -= 8;
		*reinterpret_cast<std::uint64_t*>(m_State.m_RSP) = qword;
	}

	std::uint8_t Fiber::popb()
	{
		std::uint8_t v = *reinterpret_cast<std::uint8_t*>(m_State.m_RSP);
		m_State.m_RSP += 1;
		return v;
	}

	std::uint16_t Fiber::popw()
	{
		std::uint16_t v = *reinterpret_cast<std::uint16_t*>(m_State.m_RSP);
		m_State.m_RSP += 2;
		return v;
	}

	std::uint32_t Fiber::popd()
	{
		std::uint32_t v = *reinterpret_cast<std::uint32_t*>(m_State.m_RSP);
		m_State.m_RSP += 4;
		return v;
	}

	std::uint64_t Fiber::popq()
	{
		std::uint64_t v = *reinterpret_cast<std::uint64_t*>(m_State.m_RSP);
		m_State.m_RSP += 8;
		return v;
	}
} // namespace Fibers