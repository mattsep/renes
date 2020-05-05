#pragma once

#include "nes/common.hpp"
#include "nes/locations.hpp"
#include "nes/main_bus.hpp"
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

  Cpu() { Reset(); }

  void AttachBus(MainBus* bus) { m_bus = AssumeNotNull(bus); }

  void Reset() {
    m_reg.a = 0;
    m_reg.x = 0;
    m_reg.y = 0;
    m_reg.s = 0xFF;
    m_reg.p = 0x20;
    m_reg.pc = locations::reset_vector;

    m_opinfo = optable[0];  // BRK instruction
    m_cycles = m_opinfo.cycles;
  }

  void Step() {
    // TODO: Handle interrupts
    switch (m_cycles) {
    case 2: PrepareInstruction(); break;
    case 1:
      (this->*m_op)(m_addr, m_data);
      ;
      break;
    case 0: Decode(); break;
    }

    --m_cycles;
  }

  auto GetRegisters() const -> Registers const& { return m_reg; }

  auto GetOpInfo() -> OpInfo { return m_opinfo; }

  auto GetOpAssembly() -> string {
    auto assembly = string{};
    assembly.reserve(16);
    assembly += Hexify(m_opinfo.address) + ' ' + m_opinfo.name + ' ';

    auto lo = Read(m_opinfo.address + 1);
    auto hi = Read(m_opinfo.address + 2);
    auto addr = JoinBytes(lo, hi);

    switch (m_opinfo.mode) {
    case OpMode::Absolute: assembly += Hexify(addr); break;
    case OpMode::AbsoluteX: assembly += Hexify(addr) + ",X"; break;
    case OpMode::AbsoluteY: assembly += Hexify(addr) + ",Y"; break;
    case OpMode::Immediate: assembly += '#' + Hexify(addr); break;
    case OpMode::Implied: assembly += "(imp)"; break;
    case OpMode::Indirect: assembly += '(' + Hexify(lo) + ')'; break;
    case OpMode::IndirectX: assembly += '(' + Hexify(lo) + ",X)"; break;
    case OpMode::IndirectY: assembly += '(' + Hexify(lo) + "),Y"; break;
    case OpMode::Relative: [[fallthrough]];
    case OpMode::ZeroPage: assembly += Hexify(lo); break;
    case OpMode::ZeroPageX: assembly += Hexify(lo) + ",X"; break;
    case OpMode::ZeroPageY: assembly += Hexify(lo) + ",Y"; break;
    default: throw std::runtime_error("The impossible has happened");
    }

    return assembly;
  }

private:
  using Op = void (Cpu::*)(addr_t, byte_t);

  Registers m_reg = {};
  MainBus* m_bus = nullptr;
  OpInfo m_opinfo = {};
  Op m_op = nullptr;
  addr_t m_addr = 0;
  byte_t m_data = 0;
  byte_t m_cycles = 0;

  // --------------------------------------------
  // Basic read/write operations
  // --------------------------------------------

  void Decode() {
    m_opinfo = optable[Fetch()];
    m_opinfo.address = m_reg.pc - 1;

    m_cycles = m_opinfo.cycles;

    LOG_TRACE("[CPU] Instruction: " + GetOpAssembly());
  }

  auto Read(addr_t addr) -> byte_t { return m_bus->Read(addr); }
  void Write(addr_t addr, byte_t value) { m_bus->Write(addr, value); }
  auto Fetch() -> byte_t { return m_bus->Read(m_reg.pc++); }

  auto ReadAddress(addr_t addr) -> addr_t { return JoinBytes(Read(addr), Read(addr + 1)); }

  void Push(byte_t a) {
    Write(0x0100 + m_reg.s, a);
    --m_reg.s;
  }

  auto Pull() -> byte_t {
    ++m_reg.s;
    return Read(0x0100 + m_reg.s);
  }

  // --------------------------------------------
  // Status flag operations
  // --------------------------------------------

  // status flag queries
  auto Carry() const -> bool { return m_reg.p & 0x01; }
  auto Zero() const -> bool { return m_reg.p & 0x02; }
  auto IrqDisabled() const -> bool { return m_reg.p & 0x04; }
  auto DecimalMode() const -> bool { return m_reg.p & 0x08; }
  auto BreakSet() const -> bool { return m_reg.p & 0x10; }
  auto Overflow() const -> bool { return m_reg.p & 0x40; }
  auto Negative() const -> bool { return m_reg.p & 0x80; }

  // set status flag bits
  void Carry(bool set) { set ? SetBit(m_reg.p, 0) : ClearBit(m_reg.p, 0); }
  void Zero(bool set) { set ? SetBit(m_reg.p, 1) : ClearBit(m_reg.p, 1); }
  void IrqDisabled(bool set) { set ? SetBit(m_reg.p, 2) : ClearBit(m_reg.p, 2); }
  void DecimalMode(bool set) { set ? SetBit(m_reg.p, 3) : ClearBit(m_reg.p, 3); }
  void BreakSet(bool set) { set ? SetBit(m_reg.p, 4) : ClearBit(m_reg.p, 4); }
  void Overflow(bool set) { set ? SetBit(m_reg.p, 6) : ClearBit(m_reg.p, 6); }
  void Negative(bool set) { set ? SetBit(m_reg.p, 7) : ClearBit(m_reg.p, 7); }

  // --------------------------------------------
  // Utility and fundamental operations
  // --------------------------------------------

  auto PageCrossed(addr_t a, addr_t b) -> bool { return (a >> 8) != (b >> 8); }

  void Absolute(addr_t offset = 0) {
    addr_t a = Fetch();
    a += 0x0100 * Fetch();
    addr_t b = a + offset;
    if (m_opinfo.slow_on_page_cross && PageCrossed(a, b)) { ++m_cycles; }
    m_addr = b;
  }

  void Immediate() { m_addr = m_reg.pc++; }

  void Implied() {}

  void Indirect(addr_t x = 0, addr_t y = 0) {
    auto arg = Fetch();
    auto a = JoinBytes(Read((arg + x) & 0xFF), Read((arg + x + 1) & 0xFF));
    auto b = static_cast<addr_t>(a + y);
    if (m_opinfo.slow_on_page_cross && PageCrossed(a, b)) { ++m_cycles; }
    m_addr = b;
  }

  void Relative() {
    auto offset = static_cast<std::int8_t>(Fetch());
    m_addr = static_cast<addr_t>(m_reg.pc + offset);
  }

  void ZeroPage(addr_t offset = 0) { m_addr = (Fetch() + offset) & 0xFF; }

  void Branch(bool cond) {
    if (cond) {
      ++m_cycles;
      m_reg.pc = m_addr;
    }
  }

  void Compare(byte_t a, byte_t b) {
    Carry(a >= b);
    Zero(a == b);
    Negative(TestBit(static_cast<byte_t>(a - b), 7));
  }

  // --------------------------------------------
  // Instruction preparation
  // --------------------------------------------

  void PrepareInstruction() {
    switch (m_opinfo.mode) {
    case OpMode::Absolute: Absolute(); break;
    case OpMode::AbsoluteX: Absolute(m_reg.x); break;
    case OpMode::AbsoluteY: Absolute(m_reg.y); break;
    case OpMode::Immediate: Immediate(); break;
    case OpMode::Implied: Implied(); break;
    case OpMode::Indirect: Indirect(); break;
    case OpMode::IndirectX: Indirect(m_reg.x, 0); break;
    case OpMode::IndirectY: Indirect(0, m_reg.y); break;
    case OpMode::Relative: Relative(); break;
    case OpMode::ZeroPage: ZeroPage(); break;
    case OpMode::ZeroPageX: ZeroPage(m_reg.x); break;
    case OpMode::ZeroPageY: ZeroPage(m_reg.y); break;
    default: throw std::runtime_error("The impossible has happened");
    }
    m_data = Read(m_addr);
    SetInstruction();
  }

  void SetInstruction() {
    switch (m_opinfo.type) {
    case OpType::Adc: m_op = &Cpu::Adc; break;
    case OpType::And: m_op = &Cpu::And; break;
    case OpType::Asl: m_op = &Cpu::Asl; break;
    case OpType::Bcc: m_op = &Cpu::Bcc; break;
    case OpType::Bcs: m_op = &Cpu::Bcs; break;
    case OpType::Beq: m_op = &Cpu::Beq; break;
    case OpType::Bit: m_op = &Cpu::Bit; break;
    case OpType::Bmi: m_op = &Cpu::Bmi; break;
    case OpType::Bne: m_op = &Cpu::Bne; break;
    case OpType::Bpl: m_op = &Cpu::Bpl; break;
    case OpType::Brk: m_op = &Cpu::Brk; break;
    case OpType::Bvc: m_op = &Cpu::Bvc; break;
    case OpType::Bvs: m_op = &Cpu::Bvs; break;
    case OpType::Clc: m_op = &Cpu::Clc; break;
    case OpType::Cld: m_op = &Cpu::Cld; break;
    case OpType::Cli: m_op = &Cpu::Cli; break;
    case OpType::Clv: m_op = &Cpu::Clv; break;
    case OpType::Cmp: m_op = &Cpu::Cmp; break;
    case OpType::Cpx: m_op = &Cpu::Cpx; break;
    case OpType::Cpy: m_op = &Cpu::Cpy; break;
    case OpType::Dec: m_op = &Cpu::Dec; break;
    case OpType::Dex: m_op = &Cpu::Dex; break;
    case OpType::Dey: m_op = &Cpu::Dey; break;
    case OpType::Eor: m_op = &Cpu::Eor; break;
    case OpType::Ill: m_op = &Cpu::Ill; break;
    case OpType::Inc: m_op = &Cpu::Inc; break;
    case OpType::Inx: m_op = &Cpu::Inx; break;
    case OpType::Iny: m_op = &Cpu::Iny; break;
    case OpType::Jmp: m_op = &Cpu::Jmp; break;
    case OpType::Jsr: m_op = &Cpu::Jsr; break;
    case OpType::Lda: m_op = &Cpu::Lda; break;
    case OpType::Ldx: m_op = &Cpu::Ldx; break;
    case OpType::Ldy: m_op = &Cpu::Ldy; break;
    case OpType::Lsr: m_op = &Cpu::Lsr; break;
    case OpType::Nop: m_op = &Cpu::Nop; break;
    case OpType::Ora: m_op = &Cpu::Ora; break;
    case OpType::Pha: m_op = &Cpu::Pha; break;
    case OpType::Php: m_op = &Cpu::Php; break;
    case OpType::Pla: m_op = &Cpu::Pla; break;
    case OpType::Plp: m_op = &Cpu::Plp; break;
    case OpType::Rol: m_op = &Cpu::Rol; break;
    case OpType::Ror: m_op = &Cpu::Ror; break;
    case OpType::Rti: m_op = &Cpu::Rti; break;
    case OpType::Rts: m_op = &Cpu::Rts; break;
    case OpType::Sbc: m_op = &Cpu::Sbc; break;
    case OpType::Sec: m_op = &Cpu::Sec; break;
    case OpType::Sed: m_op = &Cpu::Sed; break;
    case OpType::Sei: m_op = &Cpu::Sei; break;
    case OpType::Sta: m_op = &Cpu::Sta; break;
    case OpType::Stx: m_op = &Cpu::Stx; break;
    case OpType::Sty: m_op = &Cpu::Sty; break;
    case OpType::Tax: m_op = &Cpu::Tax; break;
    case OpType::Tay: m_op = &Cpu::Tay; break;
    case OpType::Tsx: m_op = &Cpu::Tsx; break;
    case OpType::Txa: m_op = &Cpu::Txa; break;
    case OpType::Txs: m_op = &Cpu::Txs; break;
    case OpType::Tya: m_op = &Cpu::Tya; break;
    default: throw std::runtime_error("The impossible has happened");
    }
  }

  // --------------------------------------------
  // CPU instructions
  // --------------------------------------------

  void Adc([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    auto a = m_reg.a;
    auto b = data;
    auto c = a + b + Carry();
    m_reg.a = static_cast<byte_t>(c);

    Carry(c > 0xFF);
    Zero(m_reg.a == 0);
    Overflow((a ^ c) & (b ^ c) & 0x80);
    Negative(TestBit(m_reg.a, 7));
  }

  void And([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.a &= data;
    Zero(m_reg.a == 0);
    Negative(TestBit(m_reg.a, 7));
  }

  void Asl([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    if (m_opinfo.mode == OpMode::Implied) {
      Carry(TestBit(m_reg.a, 7));
      m_reg.a <<= 1;
      Zero(m_reg.a == 0);
      Negative(TestBit(m_reg.a, 7));
    } else {
      Carry(TestBit(data, 7));
      data <<= 1;
      Zero(data == 0);  // set if data == 0 or A == 0?
      Negative(TestBit(data, 7));
      Write(addr, data);
    }
  }

  void Bcc([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Branch(!Carry()); }
  void Bcs([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Branch(Carry()); }
  void Beq([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Branch(Zero()); }

  void Bit([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    Overflow(TestBit(data, 6));
    Negative(TestBit(data, 7));
    Zero((m_reg.a & data) == 0);
  }

  void Bmi([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Branch(Negative()); }
  void Bne([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Branch(!Zero()); }
  void Bpl([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Branch(!Negative()); }

  void Brk([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    auto [lo, hi] = SplitBytes(m_reg.pc);
    Push(hi);
    Push(lo);
    Push(m_reg.p);
    BreakSet(true);
    m_reg.pc = ReadAddress(locations::reset_vector);
  }

  void Bvc([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Branch(!Overflow()); }
  void Bvs([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Branch(Overflow()); }

  void Clc([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Carry(false); }
  void Cld([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { DecimalMode(false); }
  void Cli([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { IrqDisabled(false); }
  void Clv([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Overflow(false); }

  void Cmp([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Compare(m_reg.a, data); }
  void Cpx([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Compare(m_reg.x, data); }
  void Cpy([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Compare(m_reg.y, data); }

  void Dec([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    --data;
    Write(addr, data);
    Zero(data == 0);
    Negative(TestBit(data, 7));
  }

  void Dex([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    --m_reg.x;
    Zero(m_reg.x == 0);
    Negative(TestBit(m_reg.x, 7));
  }

  void Dey([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    --m_reg.y;
    Zero(m_reg.y == 0);
    Negative(TestBit(m_reg.y, 7));
  }

  void Eor([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.a ^= data;
    Zero(m_reg.a == 0);
    Negative(TestBit(m_reg.a, 7));
  }

  void Ill([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    throw std::runtime_error("Illegal CPU instruction");
  }

  void Inc([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    ++data;
    Zero(data == 0);
    Negative(TestBit(data, 7));
    Write(addr, data);
  }

  void Inx([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    ++m_reg.x;
    Zero(m_reg.x == 0);
    Negative(TestBit(m_reg.x, 7));
  }

  void Iny([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    ++m_reg.y;
    Zero(m_reg.y == 0);
    Negative(TestBit(m_reg.y, 7));
  }

  void Jmp([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { m_reg.pc = addr; }
  void Jsr([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    auto [lo, hi] = SplitBytes(m_reg.pc);
    Push(hi);
    Push(lo);
    lo = Fetch();
    hi = Fetch();
    m_reg.pc = JoinBytes(lo, hi);
  }

  void Lda([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.a = data;
    Zero(data == 0);
    Negative(TestBit(data, 7));
  }

  void Ldx([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.x = data;
    Zero(data == 0);
    Negative(TestBit(data, 7));
  }

  void Ldy([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.y = data;
    Zero(data == 0);
    Negative(TestBit(data, 7));
  }

  void Lsr([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    Negative(false);
    if (m_opinfo.mode == OpMode::Implied) {
      Carry(TestBit(m_reg.a, 0));
      m_reg.a >>= 1;
      Zero(m_reg.a == 0);
    } else {
      Carry(TestBit(data, 0));
      data >>= 1;
      Zero(data == 0);
      Write(addr, data);
    }
  }

  void Nop([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {}

  void Ora([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.a |= data;
    Zero(m_reg.a == 0);
    Negative(TestBit(m_reg.a, 7));
  }

  void Pha([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Push(m_reg.a); }
  void Php([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Push(m_reg.p); }
  void Pla([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { m_reg.a = Pull(); }
  void Plp([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { m_reg.p = Pull(); }

  void Rol([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    byte_t c = Carry();
    if (m_opinfo.mode == OpMode::Implied) {
      Carry(TestBit(m_reg.a, 7));
      m_reg.a <<= 1;
      m_reg.a += c;
      Zero(m_reg.a == 0);
    } else {
      Carry(TestBit(data, 7));
      data <<= 1;
      data += 0x80 * c;
      Zero(data == 0);
      Write(addr, data);
    }
  }

  void Ror([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    byte_t c = Carry();
    if (m_opinfo.mode == OpMode::Implied) {
      Carry(TestBit(m_reg.a, 0));
      m_reg.a >>= 1;
      m_reg.a += c;
      Zero(m_reg.a == 0);
    } else {
      Carry(TestBit(data, 0));
      data <<= 1;
      data += 0x80 * c;
      Zero(data == 0);
      Write(addr, data);
    }
  }

  void Rti([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.p = Pull();
    auto lo = Pull();
    auto hi = Pull();
    m_reg.pc = JoinBytes(lo, hi);
  }

  void Rts([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    auto lo = Pull();
    auto hi = Pull();
    m_reg.pc = JoinBytes(lo, hi);
  }

  void Sbc([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Adc(addr, ~data); }

  void Sec([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Carry(true); }
  void Sed([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { DecimalMode(true); }
  void Sei([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { IrqDisabled(true); }

  void Sta([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Write(addr, m_reg.a); }
  void Stx([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Write(addr, m_reg.x); }
  void Sty([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { Write(addr, m_reg.y); }

  void Tax([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.x = m_reg.a;
    Zero(m_reg.x == 0);
    Negative(TestBit(m_reg.x, 7));
  }

  void Tay([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.y = m_reg.a;
    Zero(m_reg.y == 0);
    Negative(TestBit(m_reg.y, 7));
  }

  void Tsx([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.x = m_reg.s;
    Zero(m_reg.x == 0);
    Negative(TestBit(m_reg.x, 7));
  }
  void Txa([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.a = m_reg.x;
    Zero(m_reg.a == 0);
    Negative(TestBit(m_reg.a, 7));
  }

  void Txs([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) { m_reg.s = m_reg.x; }
  void Tya([[maybe_unused]] addr_t addr, [[maybe_unused]] byte_t data) {
    m_reg.a = m_reg.y;
    Zero(m_reg.a == 0);
    Negative(TestBit(m_reg.a, 7));
  }
};

}  // namespace nes