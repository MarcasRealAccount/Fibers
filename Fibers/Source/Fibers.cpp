#include "Fibers/Fibers.h"

#include <vector>

#include "Fibers/MSABI/MSABIFibers.h"
#include "Fibers/SYSV/SYSVABIFibers.h"

namespace Fibers
{
	thread_local std::vector<Fiber*> s_Fibers;

	void yield()
	{
		if (!s_Fibers.empty())
			s_Fibers.back()->yield();
	}

	void exitFiber()
	{
		if (!s_Fibers.empty())
			s_Fibers.back()->exit();
	}

	Fiber::Fiber()
	    : m_Stack(nullptr), m_StackSize(0), m_Finished(true), m_Entry(true), m_Destructor(nullptr) {}

	Fiber::Fiber(Fiber&& move) noexcept
	    : m_Stack(move.m_Stack),
	      m_StackSize(move.m_StackSize),
	      m_Finished(move.m_Finished),
	      m_Entry(move.m_Entry),
	      m_Destructor(move.m_Destructor),
	      m_EntryState(move.m_EntryState),
	      m_State(move.m_State),
	      m_ReturnState(move.m_ReturnState)
	{
		move.m_Stack = nullptr;
	}

	Fiber::~Fiber()
	{
		if (m_Stack)
			delete[] m_Stack;
	}

	void Fiber::yield()
	{
		if (!m_Stack || m_Finished)
			return;

		s_Fibers.pop_back();
		storeAndRestoreFiberStates(m_State, nullptr, m_ReturnState, nullptr);
	}

	void Fiber::exit()
	{
		if (!m_Stack || m_Finished)
			return;

		s_Fibers.pop_back();
		m_Finished = true;

		storeFiberStates(m_State, nullptr);
#if BUILD_IS_SYSTEM_WINDOWS
		m_State.m_RSP += 0x20;
#endif
		m_Destructor(*this);
		restoreFiberStates(m_ReturnState, nullptr);
	}

	void Fiber::resume()
	{
		if (!m_Stack || m_Finished)
			return;

		s_Fibers.push_back(this);
		if (m_Entry)
		{
			m_Entry = false;
			storeAndRestoreFiberStates(m_ReturnState, nullptr, m_State, &m_EntryState);
		}
		else
		{
			storeAndRestoreFiberStates(m_ReturnState, nullptr, m_State, nullptr);
		}
	}

	void Fiber::createStack(std::size_t stackSize)
	{
		m_StackSize   = (stackSize + 15) / 16 * 16;
		auto rss      = m_StackSize + 16;
		m_Stack       = new std::uint8_t[rss];
		auto stackTop = reinterpret_cast<std::uintptr_t>(m_Stack) + m_StackSize + (16 - reinterpret_cast<std::uintptr_t>(m_Stack) & 0xF);
		m_State.m_RBP = stackTop;
		m_State.m_RSP = stackTop;
	}

	/*void Fiber::pushb(std::uint8_t byte)
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
	}*/

	void Fiber::pushq(std::uint64_t qword)
	{
		m_State.m_RSP -= 8;
		*reinterpret_cast<std::uint64_t*>(m_State.m_RSP) = qword;
	}

	/*std::uint8_t Fiber::popb()
	{
		auto byte = *reinterpret_cast<std::uint8_t*>(m_State.m_RSP);
		m_State.m_RSP += 1;
		return byte;
	}

	std::uint16_t Fiber::popw()
	{
		auto word = *reinterpret_cast<std::uint16_t*>(m_State.m_RSP);
		m_State.m_RSP -= 2;
		return word;
	}

	std::uint32_t Fiber::popd()
	{
		auto dword = *reinterpret_cast<std::uint32_t*>(m_State.m_RSP);
		m_State.m_RSP -= 4;
		return dword;
	}

	std::uint64_t Fiber::popq()
	{
		auto qword = *reinterpret_cast<std::uint64_t*>(m_State.m_RSP);
		m_State.m_RSP -= 8;
		return qword;
	}*/
} // namespace Fibers