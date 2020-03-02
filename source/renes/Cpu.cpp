#include "renes/Cpu.hpp"

#include "renes/Bus.hpp"
#include "renes/Locations.hpp"

namespace renes {

Cpu::Cpu(Bus& bus) : bus_{bus} { Reset(); }

void Cpu::Reset() {
  a_ = 0;
  x_ = 0;
  y_ = 0;
  s_ = 0xFD;

  auto addr = locations::reset_vector; // reset vector
  
  // read lo byte
  Read(addr++);
  pc_ |= data_ << 0;

  // read hi byte
  Read(addr++);
  pc_ |= data_ << 4;
}

void Cpu::Read(u16 addr) {
  data_ = bus_.Read(addr);
}

void Cpu::Write(u16 addr) {
  bus_.Write(addr, data_);
}

}  // namespace renes