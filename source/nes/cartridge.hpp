#pragma once

#include <algorithm>
#include <fstream>
#include <tuple>
#include <vector>

#include "nes/common.hpp"
#include "nes/mappers.hpp"

namespace nes {

class Cartridge {
public:
  enum class Format {
    Unknown,
    Nes_v0,  // archaic NES format
    Nes_v1,  // iNES format
    Nes_v2,
  };

  enum class MirrorMode {
    Unknown,
    Horizontal,
    Vertical,
    FourScreen,
    Dynamic,
  };

  enum class TvSystem {
    Unknown,  // only support NTSC systems for now
    Ntsc,
  };

  struct Info {
    uint mapper_id = 0;
    uint submapper_id = 0;
    MirrorMode mirror_mode = MirrorMode::Unknown;
    TvSystem tv_system = TvSystem::Unknown;
  };

  auto Load(string const& file) -> bool;
  
  auto Valid() -> bool;

  auto GetInfo() -> Info const&;

  auto CpuRead(addr_t addr) -> byte_t;
  void CpuWrite(addr_t addr, byte_t value);

  auto PpuRead(addr_t addr) -> byte_t;
  void PpuWrite(addr_t addr, byte_t value);

private:
  Info m_info;
  std::unique_ptr<Mapper> m_mapper;

  auto Validate(std::vector<byte_t> const& contents) -> bool;
  auto ParseContents(std::vector<byte_t> const& contents) -> bool;
  auto GetFileFormat(std::vector<byte_t> const& contents) -> Format;
  auto GetMirroringMode(std::vector<byte_t> const& contents) -> MirrorMode;
  void GetMapper(std::vector<byte_t> const& contents, Format format);
  auto FillRom(std::vector<byte_t> const& contents, Format format) -> bool;
};

}  // namespace nes