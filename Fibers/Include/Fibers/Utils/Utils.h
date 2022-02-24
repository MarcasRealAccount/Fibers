#pragma once

namespace Fibers::Details
{
	template <class From, class To>
	union UBCastUnion
	{
	public:
		UBCastUnion(From&& from) : m_From(std::forward<From>(from)) {}

	public:
		From m_From;
		To   m_To;
	};

	template <class To, class From>
	To UBCast(From&& from)
	{
		return UBCastUnion<From, To>(std::forward<From>(from)).m_To;
	}
} // namespace Fibers::Details