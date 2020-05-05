#pragma once

#include <vector>

#include "nes/common.hpp"

namespace nes {

class Mapper {
public:
  virtual ~Mapper(){};

  auto Valid() -> bool { return (m_prg_rom.size() != 0) && (m_chr_rom.size() != 0); }

  void SetProgramRom(std::vector<byte_t>&& data) { m_prg_rom = std::move(data); }
  void SetProgramRam(std::vector<byte_t>&& data) { m_prg_ram = std::move(data); }
  void SetCharacterRom(std::vector<byte_t>&& data) { m_chr_rom = std::move(data); }
  void SetCharacterRam(std::vector<byte_t>&& data) { m_chr_ram = std::move(data); }

  virtual auto CpuRead(addr_t) -> byte_t = 0;
  virtual auto CpuWrite(addr_t, byte_t) -> byte_t = 0;
  virtual auto PpuRead(addr_t) -> byte_t = 0;
  virtual auto PpuWrite(addr_t, byte_t) -> byte_t = 0;

protected:
  std::vector<byte_t> m_prg_rom;
  std::vector<byte_t> m_chr_rom;
  std::vector<byte_t> m_prg_ram;
  std::vector<byte_t> m_chr_ram;
};

}  // namespace nes