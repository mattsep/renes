#pragma once

#include <array>
#include <stdexcept>

#include "nes/cartridge.hpp"
#include "nes/common.hpp"
#include "nes/locations.hpp"
#include "nes/ppu_bus.hpp"
#include "nes/utility.hpp"

namespace nes {

class MainBus {
  friend class PpuBus;

public:
  MainBus() = default;

  auto AttachCartridge(Cartridge* cartridge) { m_cartridge = AssumeNotNull(cartridge); }
  auto AttachPpuBus(PpuBus* ppu_bus) { m_ppu_bus = AssumeNotNull(ppu_bus); }

  auto Read(addr_t addr) const -> byte_t {
    auto ptr = MapAddress(addr);
    if (ptr) {
      return *ptr;
    } else {
      throw std::runtime_error{"Invalid address passed to MainBus::Read()."};
    }
  }

  void Write(addr_t addr, byte_t value) {
    auto ptr = MapAddress(addr);
    if (ptr) {
      *ptr = value;
    } else {
      throw std::runtime_error{"Invalid address passed to MainBus::Write()."};
    }
  }

private:
  PpuBus* m_ppu_bus = nullptr;
  Cartridge* m_cartridge = nullptr;
  std::array<byte_t, 0x0800> m_ram;

  auto MapAddress(addr_t addr) -> byte_t* {
    byte_t* ptr = nullptr;
    if (addr < 0x2000) {
      addr &= 0x07FF;
      ptr = &m_ram[addr];
    } else if (addr < 0x4000) {
      ptr = MapPpuAddress(addr);
    }

    return ptr;
  }

  auto MapAddress(addr_t addr) const -> byte_t const* {
    return const_cast<MainBus&>(*this).MapAddress(addr);
  }

  auto MapPpuAddress(addr_t addr) const -> byte_t* {
    assert(addr >= 0x2000 && addr < 0x4000);

    addr = 0x2000 + (addr & 0x0007);
    switch (addr) {
      case locations::ppu_ctrl:
      case locations::ppu_mask:
      case locations::ppu_status:
      case locations::oam_addr:
      case locations::oam_data:
      case locations::ppu_scroll:
      case locations::ppu_addr:
      case locations::ppu_data:
      default:
        return nullptr;
    }
  }
};

}  // namespace nes