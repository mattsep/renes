#pragma once

#include <array>

#include "renes/Common.hpp"

namespace renes {

class Bus {
public:
  byte_t Read(addr_t addr) const;
  void Write(addr_t addr, byte_t value);

private:
  std::array<byte_t, 0x0800> ram_;

  byte_t& MapAddress(addr_t addr);
  byte_t MapAddress(addr_t addr) const;
};

}  // namespace renes