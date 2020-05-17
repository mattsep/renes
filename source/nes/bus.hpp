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

  void AttachCpu(Cpu* cpu);
  void AttachPpu(Ppu* ppu);
  void AttachCartridge(Cartridge* cartridge);

  auto Read(addr_t addr) -> byte_t;
  void Write(addr_t addr, byte_t value);

  void RequestNmi() const;

private:
  Cpu* m_cpu = nullptr;
  Ppu* m_ppu = nullptr;
  Cartridge* m_cartridge = nullptr;
  std::array<byte_t, 0x0800> m_ram;

  auto ReadFromPpuRegister(addr_t addr) -> byte_t;
  void WriteToPpuRegister(addr_t addr, byte_t value);
};

}  // namespace nes