#pragma once

#include <memory>

#include "nes/cartridge.hpp"
#include "nes/cpu.hpp"
#include "nes/display.hpp"
#include "nes/main_bus.hpp"
#include "nes/ppu.hpp"

namespace nes {

class Console {
public:
  Console() {
    m_main_bus.AttachCartridge(&m_cartridge);
    m_cpu.AttachBus(&m_main_bus);
    m_ppu.AttachDisplay(&m_display);
  }

  auto Run() -> int {
    while (true) {
      m_cpu.Step();
      m_ppu.Step();
      m_ppu.Step();
      m_ppu.Step();
    }

    return 0;
  }

  // read-only access to internal components
  auto GetCpu() const -> Cpu const& { return m_cpu; }
  auto GetPpu() const -> Ppu const& { return m_ppu; }
  auto GetCartridge() const -> Cartridge const& { return m_cartridge; }
  auto GetDisplay() const -> Display const& { return m_display; }

private:
  Cpu m_cpu = {};
  Ppu m_ppu = {};
  MainBus m_main_bus = {};
  Cartridge m_cartridge = {};
  Display m_display = {};
};

}  // namespace nes