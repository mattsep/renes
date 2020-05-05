#pragma once

#include "nes/common.hpp"
#include "nes/mappers/mapper.hpp"

namespace nes {

class Mapper_000 : public Mapper {
public:
  auto CpuRead(addr_t addr) -> byte_t {
    if (addr < 0x8000) return 0;
    else {
      addr -= (addr >= 0x4000 && m_prg_rom.size() == 0x10000) ? 0xC000 : 0x8000;
      return m_prg_rom[addr];
    }
  }

  auto PpuRead(addr_t addr) -> byte_t { return m_chr_rom[addr]; }

  auto CpuWrite(addr_t, byte_t) -> byte_t { return 0; }
  auto PpuWrite(addr_t, byte_t) -> byte_t { return 0; }

  ~Mapper_000() = default;
};

}  // namespace nes