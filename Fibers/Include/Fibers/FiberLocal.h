#pragma once

#include "Fiber.h"

#include <shared_mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace Fibers
{
	struct FiberLocalBase
	{
	public:
		virtual void removeFiber(std::uint64_t id) = 0;
	};

	template <class T>
	struct FiberLocal : public FiberLocalBase
	{
	public:
		FiberLocal() = default;
		FiberLocal(FiberLocal&& move) noexcept
		    : m_Storage(std::move(move.m_Storage))
		{
			for (auto& v : m_Storage)
			{
				auto fiber = GetFiber(v.first);
				fiber->removeFiberLocal(&move);
				fiber->addFiberLocal(this);
			}
		}
		FiberLocal& operator=(FiberLocal&& move) noexcept
		{
			m_Storage = std::move(move.m_Storage);
			for (auto& v : m_Storage)
			{
				auto fiber = GetFiber(v.first);
				fiber->removeFiberLocal(&move);
				fiber->addFiberLocal(this);
			}
		}
		~FiberLocal()
		{
			for (auto& v : m_Storage)
			{
				auto fiber = GetFiber(v.first);
				fiber->removeFiberLocal(this);
			}
		}

		FiberLocal& operator=(const T& value)
		{
			auto fiber = GetCurrentFiber();
			if (!fiber)
				throw std::runtime_error("Not in fiber");

			m_Mutex.lock_shared();
			auto itr = m_Storage.find(fiber->getID());
			if (itr == m_Storage.end())
			{
				m_Mutex.unlock_shared();
				m_Mutex.lock();
				auto res = m_Storage.insert({ fiber->getID(), value });
				m_Mutex.unlock();
				if (res.second)
					fiber->addFiberLocal(this);
				res.first->second = value;
				return *this;
			}
			itr->second = value;
			m_Mutex.unlock_shared();
			return *this;
		}

		FiberLocal& operator=(T&& value)
		{
			auto fiber = GetCurrentFiber();
			if (!fiber)
				throw std::runtime_error("Not in fiber");

			m_Mutex.lock_shared();
			auto itr = m_Storage.find(fiber->getID());
			if (itr == m_Storage.end())
			{
				m_Mutex.unlock_shared();
				m_Mutex.lock();
				auto res = m_Storage.insert({ fiber->getID(), std::move(value) });
				m_Mutex.unlock();
				if (res.second)
					fiber->addFiberLocal(this);
				res.first->second = std::move(value);
				return *this;
			}
			itr->second = std::move(value);
			m_Mutex.unlock_shared();
			return *this;
		}

		operator T&()
		{
			auto fiber = GetCurrentFiber();
			if (!fiber)
				throw std::runtime_error("Not in fiber");

			m_Mutex.lock_shared();
			auto itr = m_Storage.find(fiber->getID());
			if (itr == m_Storage.end())
			{
				m_Mutex.unlock_shared();
				m_Mutex.lock();
				auto res = m_Storage.insert({ fiber->getID(), T {} });
				m_Mutex.unlock();
				if (res.second)
					fiber->addFiberLocal(this);
				return res.first->second;
			}
			m_Mutex.unlock_shared();
			return itr->second;
		}

		T* peek()
		{
			auto fiber = GetCurrentFiber();
			if (!fiber)
				return nullptr;

			std::shared_lock lock(m_Mutex);

			auto itr = m_Storage.find(fiber->getID());
			return itr != m_Storage.end() ? &itr->second : nullptr;
		}

		virtual void removeFiber(std::uint64_t id) override
		{
			std::unique_lock lock(m_Mutex);

			auto itr = m_Storage.find(id);
			if (itr != m_Storage.end())
				m_Storage.erase(itr);
		}

		auto& getStorage() const { return m_Storage; }

	private:
		std::unordered_map<std::uint64_t, T> m_Storage;

		mutable std::shared_mutex m_Mutex;
	};
} // namespace Fibers