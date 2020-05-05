#pragma once

#include <algorithm>
#include <fstream>
#include <tuple>
#include <vector>

#include "nes/common.hpp"
#include "nes/mappers.hpp"

namespace nes {

enum class CartridgeFormat {
  Unknown,
  Nes_v0,  // archaic NES format
  Nes_v1,
  Nes_v2,
};

enum class NameTableMirrorMode {
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

class Cartridge {
public:
  struct Info {
    uint mapper_id = 0;
    uint submapper_id = 0;
    NameTableMirrorMode mirror_mode = NameTableMirrorMode::Unknown;
    TvSystem tv_system = TvSystem::Unknown;
  };

  auto Valid() -> bool { return (!!m_mapper) && m_mapper->Valid(); }

  auto Load(string const& file) -> bool {
    m_mapper.reset();

    LOG_INFO("Loading NES file '" + file + '\'');

    std::vector<byte_t> contents;
    auto in = std::ifstream{file, std::ios::binary};
    if (in) {
      contents = std::vector<byte_t>(std::istreambuf_iterator<char>(in), {});
      if (contents.size() <= 16) {
        LOG_INFO("... File '" + file + "' is not a valid NES file");
        return false;
      }
    } else {
      LOG_WARN("... Could not read contents of '" + file + '\'');
      return false;
    }

    if (!Validate(contents)) {
      LOG_INFO("... File '" + file + "' is not a valid NES file");
      return false;
    }

    return ParseContents(contents) && Valid();
  }

  auto CpuRead(addr_t addr) -> byte_t { return m_mapper->CpuRead(addr); }

  void CpuWrite(addr_t addr, byte_t value) { m_mapper->CpuWrite(addr, value); }

  auto PpuRead(addr_t addr) -> byte_t { return m_mapper->PpuRead(addr); }

  void PpuWrite(addr_t addr, byte_t value) { m_mapper->PpuWrite(addr, value); }

private:
  Info m_info;
  std::unique_ptr<Mapper> m_mapper;

  auto Validate(std::vector<byte_t> const& contents) -> bool {
    return (contents[0] == 'N' && contents[1] == 'E' && contents[2] == 'S' &&
            contents[3] == '\x1A');
  }

  auto GetFileFormat(std::vector<byte_t> const& contents) -> CartridgeFormat {
    if ((contents[7] & 0x0C) == 0x00) {
      LOG_DEBUG("... Standard iNES format detected");
      return CartridgeFormat::Nes_v1;
    } else if ((contents[7] & 0x0C) == 0x08) {
      LOG_DEBUG("... NES 2.0 format detected");
      return CartridgeFormat::Nes_v2;
    } else {
      LOG_DEBUG("... Archaic iNES format detected");
      return CartridgeFormat::Nes_v0;
    }
    LOG_INFO("... Could not determine the file format!");
    return CartridgeFormat::Unknown;
  }

  auto ParseContents(std::vector<byte_t> const& contents) -> bool {
    auto format = GetFileFormat(contents);
    if (format == CartridgeFormat::Unknown) return false;

    GetMapperId(contents, format);
    if (m_mapper) {
      return FillRom(contents, format);
    } else {
      return false;
    }
  }

  void GetMapperId(std::vector<byte_t> const& contents, CartridgeFormat format) {
    m_info.mapper_id = contents[6] >> 4;
    if (format != CartridgeFormat::Nes_v0) { m_info.mapper_id |= contents[7] & 0xF0; }
    if (format == CartridgeFormat::Nes_v2) {
      m_info.mapper_id |= (contents[8] & 0x0Fu) << 8;
      m_info.submapper_id = contents[8] >> 4;
    }

    m_mapper = CreateMapper(m_info.mapper_id);
    if (!m_mapper) {
      LOG_INFO("... Unable to read NES file: unsupported mapper (" +
               std::to_string(m_info.mapper_id) + ')');
      return;
    }

    LOG_DEBUG("... Mapper ID: " + std::to_string(m_info.mapper_id));
  }

  auto FillRom(std::vector<byte_t> const& contents, CartridgeFormat format) -> bool {
    auto prg_rom_size = 0u;
    auto chr_rom_size = 0u;

    if (format != CartridgeFormat::Nes_v2) {
      prg_rom_size = contents[4] * 0x4000;
      chr_rom_size = contents[5] * 0x2000;
    } else {
      if (auto msb = contents[9]; (msb & 0x0F) != 0x0F) {
        prg_rom_size = static_cast<uint>(contents[4] + (msb << 8)) * 0x4000;
      } else {
        auto multiplier = 2 * (contents[4] & 0x03) + 1;
        auto exponent = (contents[4] >> 2);
        prg_rom_size = (1 << exponent) * multiplier;
      }

      if (auto msb = (contents[9] >> 4); (msb & 0x0F) != 0x0F) {
        chr_rom_size = static_cast<uint>(contents[5] + (msb << 8)) * 0x4000;
      } else {
        uint multiplier = 2 * (contents[5] & 0x03) + 1;
        uint exponent = (contents[5] >> 2);
        chr_rom_size = (1 << exponent) * multiplier;
      }
    }

    LOG_DEBUG("... Program ROM size = " + std::to_string(prg_rom_size >> 10) + " KiB");
    LOG_DEBUG("... Character ROM size = " + std::to_string(chr_rom_size >> 10) + " KiB");

    bool has_trainer = contents[6] & 0x08;

    auto total_size = 0x10 + 0x200 * has_trainer + prg_rom_size + chr_rom_size;
    if (total_size > contents.size()) {
      LOG_INFO("... Invalid NES file: file size does not match requested ROM size");
      return false;
    }

    std::vector<byte_t> buffer;

    auto beg = contents.begin() + (0x200 * has_trainer + 0x10);
    auto end = beg + prg_rom_size;
    std::copy(beg, end, std::back_inserter(buffer));
    m_mapper->SetProgramRom(std::move(buffer));

    buffer.clear();

    beg = end;
    end = beg + chr_rom_size;
    std::copy(beg, end, std::back_inserter(buffer));
    m_mapper->SetCharacterRom(std::move(buffer));

    return true;
  }
};

}  // namespace nes