#include "nes/bus.hpp"

namespace nes {

// ----------------------------------------------
// Public member function definitions
// ----------------------------------------------

void Bus::AttachCpu(Cpu* cpu) { m_cpu = AssumeNotNull(cpu); }
void Bus::AttachPpu(Ppu* ppu) { m_ppu = AssumeNotNull(ppu); }
void Bus::AttachCartridge(Cartridge* cartridge) { m_cartridge = AssumeNotNull(cartridge); }

auto Bus::Read(addr_t addr) -> byte_t {
  if (addr < 0x2000) {
    addr %= 0x0800;
    return m_ram[addr];
  } else if (addr < 0x4000) {
    return ReadFromPpuRegister(addr);
  } else if (addr < 0x4020) {
    // TODO: Access APU and Joystick registers
  } else {
    return m_cartridge->CpuRead(addr);
  }

  return 0;
}

void Bus::Write(addr_t addr, byte_t value) {
  if (addr < 0x2000) {
    addr &= 0x07FF;
    m_ram[addr] = value;
  } else if (addr < 0x4000) {
    WriteToPpuRegister(addr, value);
  } else if (addr < 0x4020) {
    // TODO: Access APU and Joystick registers
  } else {
    m_cartridge->CpuWrite(addr, value);
  }
}

void Bus::RequestNmi() const { m_cpu->RequestNmi(); }

// ----------------------------------------------
// Private member function definitions
// ----------------------------------------------

auto Bus::ReadFromPpuRegister(addr_t addr) -> byte_t {
  static byte_t data_buffer = 0;

  auto& reg = m_ppu->m_reg;
  byte_t data = 0;

  addr = 0x2000 + (addr % 8);
  switch (addr) {
  case locations::ppu_ctrl: data = reg.control; break;
  case locations::ppu_mask: data = reg.mask; break;
  case locations::ppu_status:
    data = reg.status;
    m_ppu->VBlank(false);
    m_ppu->SetLatch();
    break;
  case locations::oam_addr: break;
  case locations::oam_data: data = reg.oam_data; break;
  case locations::ppu_scroll: break;
  case locations::ppu_addr: break;
  case locations::ppu_data:
    // delayed read unless reading from palette memory
    data = data_buffer;
    data_buffer = m_ppu->Read(reg.v);
    data = (reg.v >= locations::palettes) ? data_buffer : data;
    break;
  default:
    LOG_ERROR("[BUS] Invalid read from PPU register (at " + Hexify(addr) + ')');
    throw std::runtime_error("[BUS] Invalid read from PPU register (at " + Hexify(addr) + ')');
  }

  return data;
}

void Bus::WriteToPpuRegister(addr_t addr, byte_t value) {
  auto& reg = m_ppu->m_reg;

  addr = 0x2000 + (addr % 8);
  switch (addr) {
  case locations::ppu_ctrl: reg.control = value; break;
  case locations::ppu_mask: reg.mask = value; break;
  case locations::ppu_status: break;
  case locations::oam_addr: reg.oam_address = value; break;
  case locations::oam_data:
    reg.oam_data = value;
    ++reg.oam_address;
    break;
  case locations::ppu_scroll: {
    auto a = value >> 3;
    auto b = value & 0x07;
    if (m_ppu->Latch()) {
      reg.t &= 0b0111'1111'1110'0000;
      reg.t |= a;
      reg.x = b;
    } else {
      reg.t &= 0b0000'1100'0001'1111;
      reg.t |= a << 5;
      reg.t |= b << 12;
    }
    m_ppu->ToggleLatch();
    break;
  }
  case locations::ppu_addr:
    if (m_ppu->Latch()) {
      value &= 0b0011'1111;
      reg.t &= 0x00FF;
      reg.t |= value << 8;
    } else {
      reg.t &= 0x7F00;
      reg.t |= value;
      reg.v = reg.t;
    }
    m_ppu->ToggleLatch();
    break;
  case locations::ppu_data:
    m_ppu->Write(reg.v, value);
    reg.v += m_ppu->IncModeY() ? 32 : 1;
    break;
  default:
    LOG_ERROR("[BUS] Invalid write to PPU register (at " + Hexify(addr) + ')');
    throw std::runtime_error("Invalid write to PPU register");
  }
}

}  // namespace nes