#include "nes/console.hpp"

namespace nes {

Console::Console() {
  m_bus.AttachCpu(&m_cpu);
  m_bus.AttachPpu(&m_ppu);
  m_bus.AttachCartridge(&m_cartridge);
  
  m_cpu.AttachBus(&m_bus);

  m_ppu.AttachBus(&m_bus);
  m_ppu.AttachCartridge(&m_cartridge);
  m_ppu.AttachDisplay(&m_display);
}

void Console::Load(string const& file) {
  Pause();
  if (m_cartridge.Load(file)) {
    m_cpu.Reset();
    Unpause();
  } else {
    LOG_INFO("Failed to load NES file");
  }
}

auto Console::Run() -> int {
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

void Console::Pause() { m_paused = true; }
void Console::Unpause() { m_paused = !m_cartridge.Valid(); }
void Console::TogglePause() { m_paused ? Unpause() : Pause(); }
void Console::PowerOff() { m_running = false; }
void Console::Reset() {
  Pause();
  m_cpu.Reset();
  m_ppu.Reset();
}

void Console::ForceCpuInitPc(addr_t pc) {
  m_cpu.SetProgramCounter(pc);
}

auto Console::GetCpu() const -> Cpu const& { return m_cpu; }
auto Console::GetPpu() const -> Ppu const& { return m_ppu; }
auto Console::GetCartridge() const -> Cartridge const& { return m_cartridge; }
auto Console::GetDisplay() const -> Display const& { return m_display; }

}  // namespace nes