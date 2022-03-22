#pragma once

#include <utility>

namespace Fibers::Utils
{
	namespace Details
	{
		template <class T>
		struct ConstType
		{
			using Type = T;
		};

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

	template <class T>
	struct ToNative : public Details::ConstType<T>
	{
	};

	template <class T>
	struct ToNative<T&> : public Details::ConstType<std::uintptr_t>
	{
	};

	template <class T>
	using ToNativeT = typename ToNative<T>::Type;

	template <class T>
	using ToNativeValReturn = std::conditional_t<std::is_reference_v<T>, ToNativeT<T>, ToNativeT<T>&&>;

	template <class T>
	[[nodiscard]] constexpr ToNativeValReturn<T> ToNativeVal(std::remove_reference_t<T>& original) noexcept
	{
		if constexpr (std::is_reference_v<T>)
			return reinterpret_cast<ToNativeT<T>>(&original);
		else
			return static_cast<ToNativeT<T>&&>(original);
	}

	template <class T>
	[[nodiscard]] constexpr ToNativeValReturn<T> ToNativeVal(std::remove_reference_t<T>&& original) noexcept
	{
		if constexpr (std::is_reference_v<T>)
			return reinterpret_cast<ToNativeT<T>>(&original);
		else
			return static_cast<ToNativeT<T>&&>(original);
	}
} // namespace Fibers::Utils