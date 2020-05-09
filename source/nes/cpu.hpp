#pragma once

#include "nes/bus.hpp"
#include "nes/common.hpp"
#include "nes/locations.hpp"
#include "nes/opinfo.hpp"
#include "nes/utility.hpp"

namespace nes {

class Cpu {
public:
  struct Registers {
    addr_t pc;  // program counter
    byte_t a;   // accumulator
    byte_t x;   // x-index
    byte_t y;   // y-index
    byte_t s;   // stack pointer
    byte_t p;   // processor status
  };

  Cpu();

  void AttachBus(Bus* bus);
  void Reset();
  void Step();

  void SetProgramCounter(addr_t pc);

  auto GetRegisters() const -> Registers const&;
  auto GetOpInfo() -> OpInfo;
  auto GetOpAssembly() -> string;

private:
  using Op = void (Cpu::*)();

  Registers m_reg = {};
  Bus* m_bus = nullptr;
  OpInfo m_opinfo = {};
  Op m_op = nullptr;
  addr_t m_addr = 0;
  byte_t m_data = 0;
  byte_t m_cycles = 0;
  bool m_executed = false;
  bool m_nmi = false;
  bool m_irq = false;

  // --------------------------------------------
  // Basic read/write operations
  // --------------------------------------------

  auto PrintStatus() -> string;

  void Decode();
  void Execute();

  void RequestNmi();
  void HandleIrq();
  void HandleNmi();
  void HandleReset();

  auto Read(addr_t addr) -> byte_t;
  void Write(addr_t addr, byte_t value);
  auto Fetch() -> byte_t;
  auto ReadAddress(addr_t addr) -> addr_t;
  void Push(byte_t a);
  auto Pull() -> byte_t;

  // --------------------------------------------
  // Status flag operations
  // --------------------------------------------

  // status flag queries
  auto Carry() const -> bool;
  auto Zero() const -> bool;
  auto IrqDisabled() const -> bool;
  auto DecimalMode() const -> bool;
  auto BreakSet() const -> bool;
  auto Overflow() const -> bool;
  auto Negative() const -> bool;

  // set status flag bits
  void Carry(bool set);
  void Zero(bool set);
  void IrqDisabled(bool set);
  void DecimalMode(bool set);
  void BreakSet(bool set);
  void Overflow(bool set);
  void Negative(bool set);

  // --------------------------------------------
  // Utility and fundamental operations
  // --------------------------------------------

  auto PageCrossed(addr_t a, addr_t b) -> bool;
  void Branch(bool cond);
  void Compare(byte_t a, byte_t b);
  void Interrupt(addr_t addr);

  void Absolute(addr_t offset);
  void Immediate();
  void Implied();
  void Indirect();
  void IndirectIndexed(addr_t x, addr_t y);
  void Relative();
  void ZeroPage(addr_t offset);

  // --------------------------------------------
  // Instruction preparation
  // --------------------------------------------

  void PrepareInstruction();
  void SetInstruction();

  // --------------------------------------------
  // CPU instructions
  // --------------------------------------------

  void Adc();
  void And();
  void Asl();
  void Bcc();
  void Bcs();
  void Beq();
  void Bit();
  void Bmi();
  void Bne();
  void Bpl();
  void Brk();
  void Bvc();
  void Bvs();
  void Clc();
  void Cld();
  void Cli();
  void Clv();
  void Cmp();
  void Cpx();
  void Cpy();
  void Dec();
  void Dex();
  void Dey();
  void Eor();
  void Ill();
  void Inc();
  void Inx();
  void Iny();
  void Jmp();
  void Jsr();
  void Lda();
  void Ldx();
  void Ldy();
  void Lsr();
  void Nop();
  void Ora();
  void Pha();
  void Php();
  void Pla();
  void Plp();
  void Rol();
  void Ror();
  void Rti();
  void Rts();
  void Sbc();
  void Sec();
  void Sed();
  void Sei();
  void Sta();
  void Stx();
  void Sty();
  void Tax();
  void Tay();
  void Tsx();
  void Txa();
  void Txs();
  void Tya();
};

}  // namespace nes