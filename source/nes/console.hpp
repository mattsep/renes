#pragma once

#include <chrono>
#include <memory>
#include <thread>

#include "nes/bus.hpp"
#include "nes/cartridge.hpp"
#include "nes/cpu.hpp"
#include "nes/display.hpp"
#include "nes/ppu.hpp"

namespace nes {

class Console {
public:
  Console();

  void Load(string const& file);

  auto Run() -> int;
  void Pause();
  void Unpause();
  void TogglePause();
  void PowerOff();
  void Reset();

  // read-only access to internal components
  auto GetCpu() const -> Cpu const&;
  auto GetPpu() const -> Ppu const&;
  auto GetCartridge() const -> Cartridge const&;
  auto GetDisplay() const -> Display const&;

private:
  bool m_running = true;
  bool m_paused = true;
  Bus m_bus = {};
  Cpu m_cpu = {};
  Ppu m_ppu = {};
  Cartridge m_cartridge = {};
  Display m_display = {};
};

}  // namespace nes