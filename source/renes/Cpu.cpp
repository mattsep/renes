#include "renes/Cpu.hpp"

#include "renes/Bus.hpp"
#include "renes/Locations.hpp"

namespace renes {

byte_t Cpu::CpuStatus::Pack()
{
  return (carry << 0) | (zero << 1) | (irq_disabled << 2) | (decimal_mode << 3) |
         (software_interrupt << 4) | (unused << 5) | (overflow << 6) | (negative << 7);
}

void Cpu::CpuStatus::Unpack(byte_t value)
{
  carry = value & (1 << 0);
  zero = value & (1 << 1);
  irq_disabled = value & (1 << 2);
  decimal_mode = value & (1 << 3);
  software_interrupt = value & (1 << 4);
  unused = value & (1 << 5);
  overflow = value & (1 << 6);
  negative = value & (1 << 7);
}

Cpu::Cpu(Bus& bus) : bus_{bus} { Reset(); }

void Cpu::Reset()
{
  flags_.Unpack(0x34);

  a_ = 0;
  x_ = 0;
  y_ = 0;
  s_ = 0xFD;

  auto addr = locations::reset_vector;
  pc_ |= Read(addr++) << 0;
  pc_ |= Read(addr++) << 4;
}

void Cpu::Step()
{
  switch (state_) {
  case CpuState::Decode: break;
  case CpuState::Read: break;
  case CpuState::Write: break;
  default: break;
  }
}

Cpu::CpuStatus Cpu::GetStatus() const { return flags_; }

void Cpu::Decode() {}

byte_t Cpu::Read(addr_t addr) { return bus_.Read(addr); }

void Cpu::Write(addr_t addr, byte_t value) { bus_.Write(addr, value); }

// --------------------------------------------
// Private functions
// --------------------------------------------

}  // namespace renes