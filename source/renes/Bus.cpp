#include "renes/Bus.hpp"

#include <stdexcept>

namespace renes {

byte_t Bus::Read(addr_t addr) const {
  return MapAddress(addr);
}

void Bus::Write(addr_t addr, byte_t value) {
  MapAddress(addr) = value;
}

byte_t& Bus::MapAddress(addr_t addr) {
  if (addr < 0x2000) {
    addr &= 0x07FF;
    return ram_[addr];
  } else {
    throw std::runtime_error("invalid address on main bus");
  }
}

byte_t Bus::MapAddress(addr_t addr) const {
  return const_cast<Bus&>(*this).MapAddress(addr);
}

}  // namespace renes