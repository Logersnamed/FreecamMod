#pragma once
#include <type_traits>

template <typename Flag_t>
struct Flags {
	static_assert(std::is_enum_v<Flag_t>, "Flags require enum type");

	using T = std::underlying_type_t<Flag_t>;
	static_assert(std::is_unsigned_v<T>, "Flags require unsigned underlying type");

	T flags{};

	Flags(Flag_t flag) {
		flags = static_cast<T>(flag);
	};

	constexpr bool get(Flag_t flag) const noexcept {
		return (flags & static_cast<T>(flag)) != 0;
	}

	constexpr void set(Flag_t flag, bool value = true) noexcept {
		if (value) {
			flags |= static_cast<T>(flag);
		}
		else {
			flags &= ~static_cast<T>(flag);
		}
	}

	constexpr void clear() noexcept {
		flags = 0;
	}
};

template <typename Flag_t>
[[nodiscard]] constexpr Flag_t operator&(Flag_t a, Flag_t b) noexcept {
	using T = std::underlying_type_t<Flag_t>;
	return static_cast<Flag_t>(static_cast<T>(a) & static_cast<T>(b));
}

template <typename Flag_t>
[[nodiscard]] constexpr Flag_t operator|(Flag_t a, Flag_t b) noexcept {
	using T = std::underlying_type_t<Flag_t>;
	return static_cast<Flag_t>(static_cast<T>(a) | static_cast<T>(b));
}

template <typename Flag_t>
[[nodiscard]] constexpr Flag_t operator~(Flag_t a) noexcept {
	using T = std::underlying_type_t<Flag_t>;
	return static_cast<Flag_t>(~static_cast<T>(a));
}

template <typename Flag_t>
constexpr Flag_t& operator|=(Flag_t& a, Flag_t b) noexcept {
	return a = a | b;
}

template <typename Flag_t>
constexpr Flag_t& operator&=(Flag_t& a, Flag_t b) noexcept {
	return a = a & b;
}