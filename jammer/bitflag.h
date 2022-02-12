#ifndef __BITFLAG_H__
#define __BITFLAG_H__

#include <type_traits>
#include <bitset>

// tanks a lot to the kind soul here https://softwareengineering.stackexchange.com/questions/194412/using-scoped-enums-for-bit-flags-in-c

template<typename Enum, bool IsEnum = std::is_enum<Enum>::value>
class bitflag;

template<typename Enum>
class bitflag<Enum, true>
{
public:
	constexpr const static int number_of_bits = std::numeric_limits<typename std::underlying_type<Enum>::type>::digits;

	constexpr bitflag() = default;
	constexpr bitflag(Enum value) : bits(1i64 << static_cast<std::size_t>(value)) {}
	constexpr bitflag(const bitflag& other) : bits(other.bits) {}

	constexpr bitflag operator|(Enum value) const { bitflag result = *this; result.bits |= 1i64 << static_cast<std::size_t>(value); return result; }
	constexpr bitflag operator&(Enum value) const { bitflag result = *this; result.bits &= 1i64 << static_cast<std::size_t>(value); return result; }
	constexpr bitflag operator^(Enum value) const { bitflag result = *this; result.bits ^= 1i64 << static_cast<std::size_t>(value); return result; }
	constexpr bitflag operator~() const { bitflag result = *this; result.bits.flip(); return result; }

	constexpr bitflag& operator|=(Enum value) { bits |= 1i64 << static_cast<std::size_t>(value); return *this; }
	constexpr bitflag& operator&=(Enum value) { bits &= 1i64 << static_cast<std::size_t>(value); return *this; }
	constexpr bitflag& operator^=(Enum value) { bits ^= 1i64 << static_cast<std::size_t>(value); return *this; }

	constexpr bool any() const { return bits.any(); }
	constexpr bool all() const { return bits.all(); }
	constexpr bool none() const { return bits.none(); }
	constexpr operator bool() const { return any(); }

	constexpr bool test(Enum value) const { return bits.test(static_cast<std::size_t>(value)); }
	constexpr void set(Enum value) { bits.set(static_cast<std::size_t>(value)); }
	constexpr void unset(Enum value) { bits.reset(static_cast<std::size_t>(value)); }

private:
	std::bitset<number_of_bits> bits;
};

template<typename Enum>
constexpr typename std::enable_if<std::is_enum<Enum>::value, bitflag<Enum>>::type operator|(Enum left, Enum right)
{
	return bitflag<Enum>(left) | right;
}
template<typename Enum>
constexpr typename std::enable_if<std::is_enum<Enum>::value, bitflag<Enum>>::type operator&(Enum left, Enum right)
{
	return bitflag<Enum>(left) & right;
}
template<typename Enum>
constexpr typename std::enable_if_t<std::is_enum<Enum>::value, bitflag<Enum>>::type operator^(Enum left, Enum right)
{
	return bitflag<Enum>(left) ^ right;
}

#endif //__BITFLAG_H__