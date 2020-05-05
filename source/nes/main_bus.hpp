#pragma once

#include <array>
#include <stdexcept>

#include "nes/cartridge.hpp"
#include "nes/common.hpp"
#include "nes/locations.hpp"
#include "nes/ppu.hpp"
#include "nes/utility.hpp"

namespace nes {

class MainBus {
public:
  MainBus() = default;

  auto AttachCartridge(Cartridge* cartridge) { m_cartridge = AssumeNotNull(cartridge); }
  auto AttachPpu(Ppu* ppu) { m_ppu = AssumeNotNull(ppu); }

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
    case locations::ppu_scroll: return m_ppu->m_reg.scroll;
    case locations::ppu_addr: return m_ppu->m_reg.address;
    case locations::ppu_data: return m_ppu->m_reg.data;
    default: throw std::runtime_error("The impossible has happened");
    }
  }

  void WriteToPpuRegister(addr_t addr, byte_t value) {
    addr = 0x2000 + (addr % 8);
    switch (addr) {
    case locations::ppu_ctrl: m_ppu->m_reg.control = value; break;
    case locations::ppu_mask: m_ppu->m_reg.mask = value; break;
    case locations::ppu_status: m_ppu->m_reg.status = value; break;
    case locations::oam_addr: m_ppu->m_reg.oam_address = value; break;
    case locations::oam_data: m_ppu->m_reg.oam_data = value; break;
    case locations::ppu_scroll: m_ppu->m_reg.scroll = value; break;
    case locations::ppu_addr: m_ppu->m_reg.address = value; break;
    case locations::ppu_data: m_ppu->m_reg.data = value; break;
    default: throw std::runtime_error("The impossible has happened");
    }
  }
};

}  // namespace nes