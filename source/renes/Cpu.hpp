#pragma once

#include "renes/Common.hpp"
#include "renes/Utility.hpp"

namespace renes {

class Cpu {
public:
  struct CpuInfo;

  enum class CpuState : u8 {
    Unknown,
    Decode,
    Read,
    Write,
  };

  Cpu(Bus&);

  void Reset();

  void Step();

  CpuInfo ProbeState() const;

private:
  using MicroOp = void (Cpu::*)(u16);

  // --------------------------------------------
  // Private member variables
  // --------------------------------------------

  // devices
  Bus& bus_;

  // registers
  u16 pc_;  // program counter
  u8 a_;    // accumulator
  u8 x_;    // x-index register
  u8 y_;    // y-index register
  u8 s_;    // stack pointer
  u8 p_;    // processor status flags

  // current state
  u8 opcode_;
  u8 data_;
  CpuState state_;
  MicroOp instruction_seq_[16] = {&Decode};

  // --------------------------------------------
  // Private functions
  // --------------------------------------------

  void SetCarry(bool set);
  void SetZero(bool set);
  void SetIrqDisabled(bool set);
  void SetDecimalMode(bool set);
  void SetSoftwareInterrupt(bool set);
  void SetOverflow(bool set);
  void SetNegative(bool set);
  
  bool Carry();
  bool Zero();
  bool IrqDisabled();
  bool DecimalMode();
  bool SoftwareInterrupt();
  bool Overflow();
  bool Negative();

  void Decode(u16);
  void Read(u16 addr);
  void Write(u16 addr);

};

}  // namespace renes