#include "nes/cpu.hpp"

namespace nes {

// ----------------------------------------------
// Public member function definitions
// ----------------------------------------------

Cpu::Cpu() { Reset(); }

void Cpu::AttachBus(Bus* bus) { m_bus = AssumeNotNull(bus); }

void Cpu::Reset() {
  m_opinfo = optable[0];  // BRK instruction
  m_cycles = m_opinfo.cycles;
  m_op = &Cpu::HandleReset;
}

void Cpu::Step() {
  // TODO: Handle interrupts
  switch (m_cycles) {
  case 1: Execute(); break;
  case 0: Decode(); break;
  }

  --m_cycles;
}

void Cpu::SetProgramCounter(addr_t pc) {
  HandleReset();
  m_reg.pc = pc;
  m_cycles = 0;
}

auto Cpu::GetRegisters() const -> Registers const& { return m_reg; }
auto Cpu::GetOpInfo() -> OpInfo { return m_opinfo; }
auto Cpu::GetOpAssembly() -> string {
  auto assembly = string{};
  assembly.reserve(16);
  assembly += Hexify(m_opinfo.address) + ' ' + m_opinfo.name + ' ';

  auto lo = Read(static_cast<addr_t>(m_opinfo.address + 1));
  auto hi = Read(static_cast<addr_t>(m_opinfo.address + 2));
  auto addr = JoinBytes(lo, hi);

  switch (m_opinfo.mode) {
  case OpMode::Absolute: assembly += Hexify(addr); break;
  case OpMode::AbsoluteX: assembly += Hexify(addr) + ",X"; break;
  case OpMode::AbsoluteY: assembly += Hexify(addr) + ",Y"; break;
  case OpMode::Immediate: assembly += '#' + Hexify(lo); break;
  case OpMode::Implied: assembly += "(imp)"; break;
  case OpMode::Indirect: assembly += '(' + Hexify(addr) + ')'; break;
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

// ----------------------------------------------
// Private member function definitions
// ----------------------------------------------

auto Cpu::PrintStatus() -> string {
  auto status = string{"[CPU] ... "};
  status += GetOpAssembly();
  if (status.length() <= 26) {
    status += string(26 - status.length(), ' ');
  }
  status += " | A: " + Hexify(m_reg.a);
  status += " | X: " + Hexify(m_reg.x);
  status += " | Y: " + Hexify(m_reg.y);
  status += " | P: " + Hexify(m_reg.p);
  status += " | S: " + Hexify(m_reg.s);
  return status;
}

void Cpu::Decode() {
  if (m_nmi) {
    m_opinfo = optable[0];  // BRK
    m_op = &Cpu::HandleNmi;
  } else if (m_irq && !IrqDisabled()) {
    m_opinfo = optable[0];  // BRK
    m_op = &Cpu::HandleIrq;
  } else {
    m_opinfo = optable[Fetch()];
    m_opinfo.address = m_reg.pc - 1;
    PrepareInstruction();
  }

  m_cycles = m_opinfo.cycles;
  m_executed = false;

  LOG_TRACE(PrintStatus());
}

void Cpu::Execute() {
  if (!m_executed) { (this->*m_op)(); }
  m_executed = true;
}

void Cpu::HandleIrq() {
  LOG_TRACE("[CPU] ... Handling IRQ");
  IrqDisabled(true);
  Interrupt(locations::irq_vector);
  m_irq = false;
}

void Cpu::HandleNmi() {
  LOG_TRACE("[CPU] ... Handling NMI");
  IrqDisabled(true);
  Interrupt(locations::nmi_vector);
  m_nmi = false;
}

void Cpu::HandleReset() {
  m_reg.a = 0;
  m_reg.x = 0;
  m_reg.y = 0;
  m_reg.s = 0xFD;
  m_reg.p = 0x24;
  m_reg.pc = ReadAddress(locations::reset_vector);
}

auto Cpu::Read(addr_t addr) -> byte_t { return m_bus->Read(addr); }
void Cpu::Write(addr_t addr, byte_t value) { m_bus->Write(addr, value); }
auto Cpu::Fetch() -> byte_t { return m_bus->Read(m_reg.pc++); }
auto Cpu::ReadAddress(addr_t addr) -> addr_t { return JoinBytes(Read(addr), Read(addr + 1)); }

void Cpu::Push(byte_t a) {
  Write(0x0100 + m_reg.s, a);
  --m_reg.s;
}

auto Cpu::Pull() -> byte_t {
  ++m_reg.s;
  return Read(0x0100 + m_reg.s);
}

auto Cpu::Carry() const -> bool { return m_reg.p & 0x01; }
auto Cpu::Zero() const -> bool { return m_reg.p & 0x02; }
auto Cpu::IrqDisabled() const -> bool { return m_reg.p & 0x04; }
auto Cpu::DecimalMode() const -> bool { return m_reg.p & 0x08; }
auto Cpu::BreakSet() const -> bool { return m_reg.p & 0x10; }
auto Cpu::Overflow() const -> bool { return m_reg.p & 0x40; }
auto Cpu::Negative() const -> bool { return m_reg.p & 0x80; }
void Cpu::Carry(bool set) { set ? SetBit(m_reg.p, 0) : ClearBit(m_reg.p, 0); }
void Cpu::Zero(bool set) { set ? SetBit(m_reg.p, 1) : ClearBit(m_reg.p, 1); }
void Cpu::IrqDisabled(bool set) { set ? SetBit(m_reg.p, 2) : ClearBit(m_reg.p, 2); }
void Cpu::DecimalMode(bool set) { set ? SetBit(m_reg.p, 3) : ClearBit(m_reg.p, 3); }
void Cpu::BreakSet(bool set) { set ? SetBit(m_reg.p, 4) : ClearBit(m_reg.p, 4); }
void Cpu::Overflow(bool set) { set ? SetBit(m_reg.p, 6) : ClearBit(m_reg.p, 6); }
void Cpu::Negative(bool set) { set ? SetBit(m_reg.p, 7) : ClearBit(m_reg.p, 7); }

auto Cpu::PageCrossed(addr_t a, addr_t b) -> bool { return (a >> 8) != (b >> 8); }

void Cpu::Branch(bool cond) {
  if (cond) {
    ++m_cycles;
    m_reg.pc = m_addr;
  }
}

void Cpu::Compare(byte_t a, byte_t b) {
  Carry(a >= b);
  Zero(a == b);
  Negative(TestBit(static_cast<byte_t>(a - b), 7));
}

void Cpu::Interrupt(addr_t addr) {
  auto [lo, hi] = SplitBytes(m_reg.pc);
  Push(hi);
  Push(lo);
  Push(m_reg.p);
  m_reg.pc = ReadAddress(addr);
  LOG_TRACE("[CPU] ... Program counter set to " + Hexify(m_reg.pc));
}

void Cpu::Absolute(addr_t offset = 0) {
  addr_t a = Fetch();
  a += 0x0100 * Fetch();
  addr_t b = a + offset;
  if (m_opinfo.slow_on_page_cross && PageCrossed(a, b)) { ++m_cycles; }
  m_addr = b;
}

void Cpu::Immediate() { m_addr = m_reg.pc++; }

void Cpu::Implied() {}

void Cpu::Indirect() {
  auto lo = Fetch();
  auto hi = Fetch();

  // this is to handle a cpu bug where pages can't be crossed during indirect access
  auto addr1 = JoinBytes(lo++, hi);
  auto addr2 = JoinBytes(lo++, hi);

  lo = Read(addr1);
  hi = Read(addr2);
  m_addr = JoinBytes(lo, hi);
}

void Cpu::IndirectIndexed(addr_t x = 0, addr_t y = 0) {
  auto arg = Fetch();
  auto a = JoinBytes(Read((arg + x) & 0xFF), Read((arg + x + 1) & 0xFF));
  auto b = static_cast<addr_t>(a + y);
  if (m_opinfo.slow_on_page_cross && PageCrossed(a, b)) { ++m_cycles; }
  m_addr = b;
}

void Cpu::Relative() {
  auto offset = static_cast<std::int8_t>(Fetch());
  m_addr = static_cast<addr_t>(m_reg.pc + offset);
}

void Cpu::ZeroPage(addr_t offset = 0) { m_addr = (Fetch() + offset) & 0xFF; }

void Cpu::PrepareInstruction() {
  switch (m_opinfo.mode) {
  case OpMode::Absolute: Absolute(); break;
  case OpMode::AbsoluteX: Absolute(m_reg.x); break;
  case OpMode::AbsoluteY: Absolute(m_reg.y); break;
  case OpMode::Immediate: Immediate(); break;
  case OpMode::Implied: Implied(); break;
  case OpMode::Indirect: Indirect(); break;
  case OpMode::IndirectX: IndirectIndexed(m_reg.x, 0); break;
  case OpMode::IndirectY: IndirectIndexed(0, m_reg.y); break;
  case OpMode::Relative: Relative(); break;
  case OpMode::ZeroPage: ZeroPage(); break;
  case OpMode::ZeroPageX: ZeroPage(m_reg.x); break;
  case OpMode::ZeroPageY: ZeroPage(m_reg.y); break;
  default: throw std::runtime_error("The impossible has happened");
  }
  m_data = Read(m_addr);
  SetInstruction();
}

void Cpu::SetInstruction() {
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

void Cpu::Adc() {
  auto a = m_reg.a;
  auto b = m_data;
  auto c = a + b + Carry();
  m_reg.a = static_cast<byte_t>(c);

  Carry(c > 0xFF);
  Zero(m_reg.a == 0);
  Overflow((a ^ c) & (b ^ c) & 0x80);
  Negative(TestBit(m_reg.a, 7));
}

void Cpu::And() {
  m_reg.a &= m_data;
  Zero(m_reg.a == 0);
  Negative(TestBit(m_reg.a, 7));
}

void Cpu::Asl() {
  if (m_opinfo.mode == OpMode::Implied) {
    Carry(TestBit(m_reg.a, 7));
    m_reg.a <<= 1;
    Zero(m_reg.a == 0);
    Negative(TestBit(m_reg.a, 7));
  } else {
    Carry(TestBit(m_data, 7));
    m_data <<= 1;
    Zero(m_data == 0);  // set if m_data == 0 or A == 0?
    Negative(TestBit(m_data, 7));
    Write(m_addr, m_data);
  }
}

void Cpu::Bcc() { Branch(!Carry()); }
void Cpu::Bcs() { Branch(Carry()); }
void Cpu::Beq() { Branch(Zero()); }

void Cpu::Bit() {
  Overflow(TestBit(m_data, 6));
  Negative(TestBit(m_data, 7));
  Zero((m_reg.a & m_data) == 0);
}

void Cpu::Bmi() { Branch(Negative()); }
void Cpu::Bne() { Branch(!Zero()); }
void Cpu::Bpl() { Branch(!Negative()); }

void Cpu::Brk() {
  BreakSet(true);
  Interrupt(locations::irq_vector);
}

void Cpu::Bvc() { Branch(!Overflow()); }
void Cpu::Bvs() { Branch(Overflow()); }

void Cpu::Clc() { Carry(false); }
void Cpu::Cld() { DecimalMode(false); }
void Cpu::Cli() { IrqDisabled(false); }
void Cpu::Clv() { Overflow(false); }

void Cpu::Cmp() { Compare(m_reg.a, m_data); }
void Cpu::Cpx() { Compare(m_reg.x, m_data); }
void Cpu::Cpy() { Compare(m_reg.y, m_data); }

void Cpu::Dec() {
  --m_data;
  Write(m_addr, m_data);
  Zero(m_data == 0);
  Negative(TestBit(m_data, 7));
}

void Cpu::Dex() {
  --m_reg.x;
  Zero(m_reg.x == 0);
  Negative(TestBit(m_reg.x, 7));
}

void Cpu::Dey() {
  --m_reg.y;
  Zero(m_reg.y == 0);
  Negative(TestBit(m_reg.y, 7));
}

void Cpu::Eor() {
  m_reg.a ^= m_data;
  Zero(m_reg.a == 0);
  Negative(TestBit(m_reg.a, 7));
}

void Cpu::Ill() {
  auto message = std::string{"Illegal CPU instruction: "} + Hexify(Read(m_opinfo.address)) + " [" +
                 GetOpAssembly() + "]";
  throw std::runtime_error(message);
}

void Cpu::Inc() {
  ++m_data;
  Zero(m_data == 0);
  Negative(TestBit(m_data, 7));
  Write(m_addr, m_data);
}

void Cpu::Inx() {
  ++m_reg.x;
  Zero(m_reg.x == 0);
  Negative(TestBit(m_reg.x, 7));
}

void Cpu::Iny() {
  ++m_reg.y;
  Zero(m_reg.y == 0);
  Negative(TestBit(m_reg.y, 7));
}

void Cpu::Jmp() { m_reg.pc = m_addr; }
void Cpu::Jsr() {
  auto [lo, hi] = SplitBytes(--m_reg.pc);
  Push(hi);
  Push(lo);
  m_reg.pc = m_addr;
}

void Cpu::Lda() {
  m_reg.a = m_data;
  Zero(m_data == 0);
  Negative(TestBit(m_data, 7));
}

void Cpu::Ldx() {
  m_reg.x = m_data;
  Zero(m_data == 0);
  Negative(TestBit(m_data, 7));
}

void Cpu::Ldy() {
  m_reg.y = m_data;
  Zero(m_data == 0);
  Negative(TestBit(m_data, 7));
}

void Cpu::Lsr() {
  Negative(false);
  if (m_opinfo.mode == OpMode::Implied) {
    Carry(TestBit(m_reg.a, 0));
    m_reg.a >>= 1;
    Zero(m_reg.a == 0);
  } else {
    Carry(TestBit(m_data, 0));
    m_data >>= 1;
    Zero(m_data == 0);
    Write(m_addr, m_data);
  }
}

void Cpu::Nop() {}

void Cpu::Ora() {
  m_reg.a |= m_data;
  Zero(m_reg.a == 0);
  Negative(TestBit(m_reg.a, 7));
}

void Cpu::Pha() { Push(m_reg.a); }
void Cpu::Php() { Push(m_reg.p); }
void Cpu::Pla() { m_reg.a = Pull(); }
void Cpu::Plp() { m_reg.p = Pull(); }

void Cpu::Rol() {
  byte_t c = Carry();
  if (m_opinfo.mode == OpMode::Implied) {
    Carry(TestBit(m_reg.a, 7));
    m_reg.a <<= 1;
    m_reg.a += c;
    Zero(m_reg.a == 0);
  } else {
    Carry(TestBit(m_data, 7));
    m_data <<= 1;
    m_data += 0x80 * c;
    Zero(m_data == 0);
    Write(m_addr, m_data);
  }
}

void Cpu::Ror() {
  byte_t c = Carry();
  if (m_opinfo.mode == OpMode::Implied) {
    Carry(TestBit(m_reg.a, 0));
    m_reg.a >>= 1;
    m_reg.a += c;
    Zero(m_reg.a == 0);
  } else {
    Carry(TestBit(m_data, 0));
    m_data <<= 1;
    m_data += 0x80 * c;
    Zero(m_data == 0);
    Write(m_addr, m_data);
  }
}

void Cpu::Rti() {
  m_reg.p = Pull();
  auto lo = Pull();
  auto hi = Pull();
  m_reg.pc = JoinBytes(lo, hi);
}

void Cpu::Rts() {
  auto lo = Pull();
  auto hi = Pull();
  m_reg.pc = JoinBytes(lo, hi) + 1;
}

void Cpu::Sbc() {
  m_data = ~m_data;
  Adc();
}

void Cpu::Sec() { Carry(true); }
void Cpu::Sed() { DecimalMode(true); }
void Cpu::Sei() { IrqDisabled(true); }
void Cpu::Sta() { Write(m_addr, m_reg.a); }
void Cpu::Stx() { Write(m_addr, m_reg.x); }
void Cpu::Sty() { Write(m_addr, m_reg.y); }

void Cpu::Tax() {
  m_reg.x = m_reg.a;
  Zero(m_reg.x == 0);
  Negative(TestBit(m_reg.x, 7));
}

void Cpu::Tay() {
  m_reg.y = m_reg.a;
  Zero(m_reg.y == 0);
  Negative(TestBit(m_reg.y, 7));
}

void Cpu::Tsx() {
  m_reg.x = m_reg.s;
  Zero(m_reg.x == 0);
  Negative(TestBit(m_reg.x, 7));
}
void Cpu::Txa() {
  m_reg.a = m_reg.x;
  Zero(m_reg.a == 0);
  Negative(TestBit(m_reg.a, 7));
}

void Cpu::Txs() { m_reg.s = m_reg.x; }
void Cpu::Tya() {
  m_reg.a = m_reg.y;
  Zero(m_reg.a == 0);
  Negative(TestBit(m_reg.a, 7));
}

}  // namespace nes