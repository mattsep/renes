#include "nes/cartridge.hpp"

namespace nes {

// ----------------------------------------------
// Public member function definitions
// ----------------------------------------------

auto Cartridge::Load(string const& file) -> bool {
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

auto Cartridge::Valid() -> bool { return (!!m_mapper) && (m_mapper->Valid()); }

auto Cartridge::GetInfo() -> Info const& { return m_info; }

auto Cartridge::CpuRead(addr_t addr) -> byte_t { return m_mapper->CpuRead(addr); }
void Cartridge::CpuWrite(addr_t addr, byte_t value) { m_mapper->CpuWrite(addr, value); }

auto Cartridge::PpuRead(addr_t addr) -> byte_t { return m_mapper->PpuRead(addr); }
void Cartridge::PpuWrite(addr_t addr, byte_t value) { m_mapper->PpuWrite(addr, value); }

// ----------------------------------------------
// Private member function definitions
// ----------------------------------------------

auto Cartridge::Validate(std::vector<byte_t> const& contents) -> bool {
  return (contents[0] == 'N' && contents[1] == 'E' && contents[2] == 'S' && contents[3] == '\x1A');
}

auto Cartridge::ParseContents(std::vector<byte_t> const& contents) -> bool {
  auto format = GetFileFormat(contents);
  if (format == Format::Unknown) return false;

  GetMapper(contents, format);
  m_info.mirror_mode = GetMirroringMode(contents);
  if (m_mapper) {
    return FillRom(contents, format);
  } else {
    return false;
  }
}

auto Cartridge::GetFileFormat(std::vector<byte_t> const& contents) -> Format {
  if ((contents[7] & 0x0C) == 0x00) {
    LOG_DEBUG("... Standard iNES format detected");
    return Format::Nes_v1;
  } else if ((contents[7] & 0x0C) == 0x08) {
    LOG_DEBUG("... NES 2.0 format detected");
    return Format::Nes_v2;
  } else {
    LOG_DEBUG("... Archaic iNES format detected");
    return Format::Nes_v0;
  }
  LOG_INFO("... Could not determine the file format!");
  return Format::Unknown;
}

auto Cartridge::GetMirroringMode(std::vector<byte_t> const& contents) -> MirrorMode {
  auto mode = (contents[6] & 1) ? MirrorMode::Vertical : MirrorMode::Horizontal;
  if (contents[6] & 0b1000) mode = MirrorMode::FourScreen;
  return mode;
}

void Cartridge::GetMapper(std::vector<byte_t> const& contents, Format format) {
  m_info.mapper_id = contents[6] >> 4;

  if (format == Format::Nes_v1) { m_info.mapper_id |= contents[7] & 0xF0; }

  if (format == Format::Nes_v2) {
    m_info.mapper_id |= (contents[8] & 0x0F) << 8;
    m_info.submapper_id = contents[8] >> 4;
  }

  m_mapper = CreateMapper(m_info.mapper_id);
}

auto Cartridge::FillRom(std::vector<byte_t> const& contents, Format format) -> bool {
  auto prg_rom_size = 0u;
  auto chr_rom_size = 0u;

  if (format != Format::Nes_v2) {
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

  std::vector<byte_t> buffer(prg_rom_size);

  auto beg = contents.begin() + (0x200 * has_trainer + 0x10);
  auto end = beg + prg_rom_size;
  std::copy(beg, end, buffer.begin());
  m_mapper->SetProgramRom(std::move(buffer));

  buffer.resize(chr_rom_size);

  beg = end;
  end = beg + chr_rom_size;
  std::copy(beg, end, buffer.begin());
  m_mapper->SetCharacterRom(std::move(buffer));

  return true;
}

}  // namespace nes