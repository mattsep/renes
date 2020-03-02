#pragma once

#include "renes/Common.hpp"

namespace renes {

template <class Int, class = std::enable_if_t<std::is_unsigned_v<Int>>>
string Hexify(Int n)
{
  constexpr char[] digits = "0123456789ABCDEF";

  auto result = string(1 + 2 * sizeof(Int), '0');
  result[0] = '$';

  size_t pos = result.size() - 1;
  while (n) {
    result[pos--] = digits[n & 0x0F];
    n >>= 4;
  }

  return result;
}

template <u8 Bit, class Int, class = std::enable_if_t<std::is_unsigned_v<Int>>>
constexpr Int SetBit(Int value, bool cond)
{
  return (value & ~(1 << Bit)) | (cond << Bit);
}

}  // namespace renes