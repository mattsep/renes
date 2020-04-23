#pragma once

#include <cstring>
#include <type_traits>

#include "nes/common.hpp"

namespace nes {

template <class Int, class = std::enable_if_t<std::is_unsigned_v<Int>>>
auto Hexify(Int n) -> string {
  constexpr auto digits = "0123456789ABCDEF";
  constexpr auto nchars = 1 + 2 * sizeof(Int);

  auto result = string(nchars, '0');
  result[0] = '$';

  auto pos = nchars;
  while (n) {
    result[--pos] = digits[n & 0x0F];
    n >>= 4;
  }

  return result;
}

template <class Int, class = std::enable_if_t<std::is_unsigned_v<Int>>>
constexpr auto TestBit(Int n, uint bit) -> bool {
  return static_cast<bool>((n >> bit) & 1u);
}

template <class Int, class = std::enable_if_t<std::is_unsigned_v<Int>>>
constexpr void SetBit(Int& n, uint bit) {
  n |= (Int{1} << bit);
}

template <class Int, class = std::enable_if_t<std::is_unsigned_v<Int>>>
constexpr void ClearBit(Int& n, uint bit) {
  n &= ~(Int{1} << bit);
}

template <class Int, class = std::enable_if_t<std::is_unsigned_v<Int>>>
constexpr auto SplitBytes(Int value) {
  std::array<byte_t, sizeof(Int)> bytes = {};
  for (auto i = 0u; i < sizeof(Int); ++i) {
    bytes[i] = static_cast<byte_t>(value & 0xFF);
    value >>= 8;
  }
  return bytes;
}

template <class To, class From,
          class = std::enable_if_t<(sizeof(To) == sizeof(From) && std::is_trivial_v<To> &&
                                    std::is_trivially_copyable_v<From>)>>
auto BitCast(From const& from) -> To {
  To to;
  std::memcpy(&to, &from, sizeof(to));
  return to;
}

template <class T>
constexpr auto AssumeNotNull(T* ptr) -> T* {
  assert(ptr != nullptr);
  return ptr;
}

}  // namespace nes