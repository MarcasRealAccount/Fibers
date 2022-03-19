#pragma once

#include <utility>

namespace Fibers::Utils
{
	namespace Details
	{
		template <class From, class To>
		union UBCast
		{
		public:
			UBCast(From&& from) : m_From(std::forward<From>(from)) {}

		public:
			From m_From;
			To   m_To;
		};
	} // namespace Details

	template <class To, class From>
	To UBCast(From&& from)
	{
		return Details::UBCast<From, To>(std::forward<From>(from)).m_To;
	}
} // namespace Fibers::Utils