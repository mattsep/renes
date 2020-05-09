#include "nes/mappers/mapper_000.hpp"

namespace nes {

auto Mapper_000::CpuRead(addr_t addr) -> byte_t {
  if (addr < 0x8000) return 0;
  else {
    addr -= 0x8000;
    if (addr >= m_prg_rom.size()) addr %= 0x4000;
    return m_prg_rom[addr];
  }
}

auto Mapper_000::PpuRead(addr_t addr) -> byte_t { return m_chr_rom[addr]; }
auto Mapper_000::CpuWrite(addr_t, byte_t) -> byte_t { return 0; }
auto Mapper_000::PpuWrite(addr_t, byte_t) -> byte_t { return 0; }

}  // namespace nes