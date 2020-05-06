#pragma once

#include <chrono>
#include <memory>
#include <thread>

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

  void Load(string const& file) {
    Pause();
    if (m_cartridge.Load(file)) {
      m_cpu.Reset();
      Unpause();
    } else {
      LOG_INFO("Failed to load NES file");
    }
  }

  auto Run() -> int {
    using namespace std::literals;

    while (m_running) {
      if (!m_paused) {
        m_cpu.Step();
        m_ppu.Step();
        m_ppu.Step();
        m_ppu.Step();
      } else {
        std::this_thread::sleep_for(10ms);
      }
    }

    return 0;
  }

  void Pause() { m_paused = true; }
  void Unpause() { m_paused = !m_cartridge.Valid(); }
  void PowerOff() { m_running = false; }
  void Reset() {
    Pause();
    m_cpu.Reset();
  }

  // read-only access to internal components
  auto GetCpu() const -> Cpu const& { return m_cpu; }
  auto GetPpu() const -> Ppu const& { return m_ppu; }
  auto GetCartridge() const -> Cartridge const& { return m_cartridge; }
  auto GetDisplay() const -> Display const& { return m_display; }

private:
  bool m_running = true;
  bool m_paused = true;
  Cpu m_cpu = {};
  Ppu m_ppu = {};
  MainBus m_main_bus = {};
  PpuBus m_ppu_bus = {};
  Cartridge m_cartridge = {};
  Display m_display = {};
};

}  // namespace nes