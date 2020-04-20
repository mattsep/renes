#pragma once

#include "nes/main_bus.hpp"
#include "nes/common.hpp"
#include "nes/locations.hpp"
#include "nes/utility.hpp"

namespace nes {

class Cpu {
public:
  struct Registers {
    addr_t program_counter;
    byte_t accumulator;
    byte_t x_index;
    byte_t y_index;
    byte_t stack_pointer;
    byte_t flags;
  };

  enum class CpuState : byte_t {
    Unknown,
    Decode,
    Read,
    Write,
  };

  void AttachBus(MainBus* bus) { m_bus = AssumeNotNull(bus); }

  void Reset() {
    m_reg.accumulator = 0;
    m_reg.x_index = 0;
    m_reg.y_index = 0;
    m_reg.stack_pointer = 0xFD;
    m_reg.flags = 0x20;

    auto addr = locations::reset_vector;
    m_reg.program_counter |= Read(addr++) << 0;
    m_reg.program_counter |= Read(addr++) << 4;
  }

  void Step() {}

  auto GetRegisters() const -> Registers const& { return m_reg; }

private:
  using MicroOp = void (Cpu::*)(addr_t);

  // --------------------------------------------
  // Private member variables
  // --------------------------------------------

  MainBus* m_bus;
  Registers m_reg;

  // --------------------------------------------
  // Private functions
  // --------------------------------------------

  auto Decode() {}
  auto Read(addr_t addr) -> byte_t { return m_bus->Read(addr); }
  auto Write(addr_t addr, byte_t value) { m_bus->Write(addr, value); }
};

}  // namespace nes