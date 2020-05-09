#pragma once

#include <array>
#include <stdexcept>

#include "nes/cartridge.hpp"
#include "nes/common.hpp"
#include "nes/cpu.hpp"
#include "nes/locations.hpp"
#include "nes/ppu.hpp"
#include "nes/utility.hpp"

namespace nes {

class Bus {
public:
  Bus() = default;

  void AttachCpu(Cpu* cpu) { m_cpu = AssumeNotNull(cpu); }
  void AttachPpu(Ppu* ppu) { m_ppu = AssumeNotNull(ppu); }
  void AttachCartridge(Cartridge* cartridge) { m_cartridge = AssumeNotNull(cartridge); }

  auto Read(addr_t addr) -> byte_t {
    if (addr < 0x2000) {
      addr %= 0x0800;
      return m_ram[addr];
    } else if (addr < 0x4000) {
      ReadFromPpuRegister(addr);
    } else if (addr < 0x4020) {
      // TODO: Access APU and Joystick registers
    } else {
      return m_cartridge->CpuRead(addr);
    }

    return 0;
  }

  void Write(addr_t addr, byte_t value) {
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

private:
  Cpu* m_cpu = nullptr;
  Ppu* m_ppu = nullptr;
  Cartridge* m_cartridge = nullptr;
  std::array<byte_t, 0x0800> m_ram;

  auto ReadFromPpuRegister(addr_t addr) -> byte_t {
    addr = 0x2000 + (addr % 8);
    switch (addr) {
    case locations::ppu_ctrl: return m_ppu->m_reg.control;
    case locations::ppu_mask: return m_ppu->m_reg.mask;
    case locations::ppu_status: return m_ppu->m_reg.status;
    case locations::oam_addr: return m_ppu->m_reg.oam_address;
    case locations::oam_data: return m_ppu->m_reg.oam_data;
    // case locations::ppu_scroll: not readable
    case locations::ppu_addr: return m_ppu->m_reg.address;
    case locations::ppu_data: return m_ppu->Read(m_ppu->m_reg.address);
    default: throw std::runtime_error("Invalid read from PPU register");
    }
  }

  void WriteToPpuRegister(addr_t addr, byte_t value) {
    addr = 0x2000 + (addr % 8);
    switch (addr) {
    case locations::ppu_ctrl: m_ppu->m_reg.control = value; break;

    case locations::ppu_mask:
      m_ppu->m_reg.mask = value;
      break;

      // case locations::ppu_status: not writable

    case locations::oam_addr: m_ppu->m_reg.oam_address = value; break;

    case locations::oam_data:
      m_ppu->m_reg.oam_data = value;
      ++m_ppu->m_reg.oam_address;
      break;

    case locations::ppu_scroll:
      static bool scroll_x = true;
      if (scroll_x) {
        m_ppu->m_reg.scroll_x = value;
      } else {
        m_ppu->m_reg.scroll_y = value;
      }
      scroll_x = !scroll_x;
      break;

    case locations::ppu_addr:
      static bool write_hi = true;
      if (write_hi) {
        m_ppu->m_reg.address = value << 8;
      } else {
        m_ppu->m_reg.address |= value;
      }
      write_hi = !write_hi;
      break;

    case locations::ppu_data:
      m_ppu->m_reg.data = value;
      m_ppu->m_reg.address += TestBit(m_ppu->m_reg.control, 2) ? 32 : 1;
      break;

    default: throw std::runtime_error("Invalid write to PPU register");
    }
  }
};

}  // namespace nes