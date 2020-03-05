#pragma once

#include "renes/Common.hpp"
#include "renes/Utility.hpp"

namespace renes {

class Cpu {
public:
  struct CpuStatus {
    bool carry = false;
    bool zero = false;
    bool irq_disabled = false;
    bool decimal_mode = false;
    bool software_interrupt = false;
    bool unused = true; // bit 5 is unused, but is typically set 
    bool overflow = false;
    bool negative = false;

    byte_t Pack();
    void Unpack(byte_t flags);
  };

  enum class CpuState : byte_t {
    Unknown,
    Decode,
    Read,
    Write,
  };

  Cpu(Bus&);

  void Reset();

  void Step();

  CpuStatus GetStatus() const;

private:
  using MicroOp = void (Cpu::*)(addr_t);

  // --------------------------------------------
  // Private member variables
  // --------------------------------------------

  Bus& bus_;  // main bus

  addr_t pc_;  // program counter

  byte_t a_;  // accumulator
  byte_t x_;  // x-index register
  byte_t y_;  // y-index register
  byte_t s_;  // stack pointer

  CpuStatus flags_;  // processor status flags
  CpuState state_;   // whether the CPU is in read or write mode for this cycle

  // --------------------------------------------
  // Private functions
  // --------------------------------------------

  void Decode();
  byte_t Read(addr_t addr);
  void Write(addr_t addr, byte_t value);
};

}  // namespace renes