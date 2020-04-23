#pragma once

#include <memory>

#include "nes/cartridge.hpp"
#include "nes/cpu.hpp"
#include "nes/display.hpp"
#include "nes/main_bus.hpp"
#include "nes/ppu.hpp"
#include "nes/ppu_bus.hpp"

namespace nes {

class Console {
public:
  Console() {
    m_main_bus.AttachCartridge(&m_cartridge);
    m_main_bus.AttachPpu(&m_ppu);
    m_cpu.AttachBus(&m_main_bus);
    m_ppu.AttachBus(&m_ppu_bus);
    m_ppu.AttachDisplay(&m_display);
  }

  void Load([[maybe_unused]] string filename) {}

  auto Run() -> int {
    while (m_running) {
      m_cpu.Step();
      m_ppu.Step();
      m_ppu.Step();
      m_ppu.Step();
    }

    return 0;
  }

  void PowerOff() { m_running = false; }

  // read-only access to internal components
  auto GetCpu() const -> Cpu const& { return m_cpu; }
  auto GetPpu() const -> Ppu const& { return m_ppu; }
  auto GetCartridge() const -> Cartridge const& { return m_cartridge; }
  auto GetDisplay() const -> Display const& { return m_display; }

private:
  bool m_running = true;
  Cpu m_cpu = {};
  Ppu m_ppu = {};
  MainBus m_main_bus = {};
  PpuBus m_ppu_bus = {};
  Cartridge m_cartridge = {};
  Display m_display = {};
};

}  // namespace nes