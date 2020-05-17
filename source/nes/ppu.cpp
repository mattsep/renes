#include "nes/ppu.hpp"

namespace nes {

// ----------------------------------------------
// Public member function definitions
// ----------------------------------------------

void Ppu::Reset() { m_reg = Registers{}; }

void Ppu::AttachBus(Bus* bus) { m_bus = AssumeNotNull(bus); }
void Ppu::AttachCartridge(Cartridge* cartridge) { m_cartridge = AssumeNotNull(cartridge); }
void Ppu::AttachDisplay(Display* display) { m_display = AssumeNotNull(display); }

void Ppu::Step() {
  // The PPU skips the point (340, 261) on odd frames. This is equivalent to skipping the idle
  // step at (0, 0), and letting scanline 261 be a full render line. This only happens when
  // rendering is enabled.
  if (m_row == 0 && m_col == 0) {
    if (ShowFg() || ShowBg()) { m_col = m_frame_odd ? 1 : 0; }
  }

  if (m_row < Row::screen_height || m_row == Row::pre_render) { RenderCycle(); }
  if (m_row == Row::vblank_clear && m_col == Col::vblank_clear) {
    LOG_TRACE("[PPU] Clearing VBLANK");
    VBlank(false);
  }
  if ((m_row == Row::vblank_set) && (m_col == Col::vblank_set)) {
    LOG_TRACE("[PPU] Setting VBLANK");
    VBlank(true);
    if (GenNmi()) {
      LOG_TRACE("[PPU] Requesting NMI");
      m_bus->RequestNmi();
    }
  }

  DrawPixel();
}

// ----------------------------------------------
// Private member function definitions
// ----------------------------------------------

void Ppu::DrawPixel() {
  byte_t index = 0;
  if (ShowBg()) {
    auto bit = 1 << (15 - m_reg.x);
    index |= !!(m_reg.bg_patt_shifter_lo & bit) << 0;
    index |= !!(m_reg.bg_patt_shifter_hi & bit) << 1;
    index |= !!(m_reg.bg_attr_shifter_lo & bit) << 2;
    index |= !!(m_reg.bg_attr_shifter_hi & bit) << 3;
  }

  if (ShowFg()) {
    // TODO handle sprites
  }

  addr_t addr = locations::palettes + (index & 0x1F);
  byte_t pixel = Read(addr) & 0x3F;
  auto color = pallete[pixel];
  m_display->DrawPixel(m_col - 1, m_row, color);

  // move to next pixel
  if (++m_col > Col::max) {
    m_col = 0;
    if (++m_row > Row::max) {
      m_row = 0;
      m_frame_odd = !m_frame_odd;
      LOG_TRACE("[PPU] End frame");
    }
  }
}

auto Ppu::Read(addr_t addr) const -> byte_t {
  if (addr < locations::name_table_0) {
    // auto which = addr / 0x1000;
    // addr %= 0x1000;
    // return m_pattern_tables[which][addr];
    return m_cartridge->PpuRead(addr);
  } else if (addr < locations::palettes) {
    addr %= 0x1000;
    auto which = addr / 0x0400;
    addr %= 0x0400;

    auto mirroring = m_cartridge->GetInfo().mirror_mode;
    switch (mirroring) {
    case Cartridge::MirrorMode::Horizontal: which /= 2; break;
    case Cartridge::MirrorMode::Vertical: which %= 2; break;
    case Cartridge::MirrorMode::FourScreen: break;
    default:
      LOG_ERROR("[PPU] Unknown mirroring mode detected");
      throw std::runtime_error("Unknown mirroring mode detected");
    }

    return m_name_tables[which][addr];
  } else {
    addr %= 0x0020;
    switch (addr) {
    case 0x10: [[fallthrough]];
    case 0x14: [[fallthrough]];
    case 0x18: [[fallthrough]];
    case 0x1C: addr -= 0x10; break;
    }
    return m_palette_table[addr];
  }
}

void Ppu::Write(addr_t addr, byte_t value) {
  if (addr < locations::name_table_0) {
    auto which = addr / 0x1000;
    addr %= 0x1000;
    m_pattern_tables[which][addr] = value;
  } else if (addr < locations::palettes) {
    addr %= 0x1000;
    auto which = addr / 0x0400;
    addr %= 0x0400;

    auto mirroring = m_cartridge->GetInfo().mirror_mode;
    switch (mirroring) {
    case Cartridge::MirrorMode::Horizontal: which /= 2; break;
    case Cartridge::MirrorMode::Vertical: which %= 2; break;
    case Cartridge::MirrorMode::FourScreen: break;
    default:
      LOG_ERROR("[PPU] Unknown mirroring mode detected");
      throw std::runtime_error("Unknown mirroring mode detected");
    }

    m_name_tables[which][addr] = value;
  } else {
    addr %= 0x0020;
    switch (addr) {
    case 0x10: [[fallthrough]];
    case 0x14: [[fallthrough]];
    case 0x18: [[fallthrough]];
    case 0x1C: addr -= 0x10; break;
    }
    m_palette_table[addr] = value;
    LOG_TRACE("[PPU] Writing to palette table");
  }
}

auto Ppu::NtBaseX() const -> bool { return TestBit(m_reg.control, 0); }
auto Ppu::NtBaseY() const -> bool { return TestBit(m_reg.control, 1); }
auto Ppu::IncModeY() const -> bool { return TestBit(m_reg.control, 2); }
auto Ppu::SpriteTable() const -> bool { return TestBit(m_reg.control, 3); }
auto Ppu::BgTable() const -> bool { return TestBit(m_reg.control, 4); }
auto Ppu::BigSprites() const -> bool { return TestBit(m_reg.control, 5); }
auto Ppu::ExtWrite() const -> bool { return TestBit(m_reg.control, 6); }
auto Ppu::GenNmi() const -> bool { return TestBit(m_reg.control, 7); }

void Ppu::NtBaseX(bool set) { set ? SetBit(m_reg.control, 0) : ClearBit(m_reg.control, 0); }
void Ppu::NtBaseY(bool set) { set ? SetBit(m_reg.control, 1) : ClearBit(m_reg.control, 1); }
void Ppu::IncModeY(bool set) { set ? SetBit(m_reg.control, 2) : ClearBit(m_reg.control, 2); }
void Ppu::SpriteTable(bool set) { set ? SetBit(m_reg.control, 3) : ClearBit(m_reg.control, 3); }
void Ppu::BgTable(bool set) { set ? SetBit(m_reg.control, 4) : ClearBit(m_reg.control, 4); }
void Ppu::BigSprites(bool set) { set ? SetBit(m_reg.control, 5) : ClearBit(m_reg.control, 5); }
void Ppu::ExtWrite(bool set) { set ? SetBit(m_reg.control, 6) : ClearBit(m_reg.control, 6); }
void Ppu::GenNmi(bool set) { set ? SetBit(m_reg.control, 7) : ClearBit(m_reg.control, 7); }

auto Ppu::Grayscale() const -> bool { return TestBit(m_reg.mask, 0); }
auto Ppu::ShowBgLeft() const -> bool { return TestBit(m_reg.mask, 1); };
auto Ppu::ShowFgLeft() const -> bool { return TestBit(m_reg.mask, 2); };
auto Ppu::ShowBg() const -> bool { return TestBit(m_reg.mask, 3); };
auto Ppu::ShowFg() const -> bool { return TestBit(m_reg.mask, 4); };
auto Ppu::EmphRed() const -> bool { return TestBit(m_reg.mask, 5); };
auto Ppu::EmphGreen() const -> bool { return TestBit(m_reg.mask, 6); };
auto Ppu::EmphBlue() const -> bool { return TestBit(m_reg.mask, 7); };

void Ppu::Grayscale(bool set) { set ? SetBit(m_reg.mask, 0) : ClearBit(m_reg.mask, 0); }
void Ppu::ShowBgLeft(bool set) { set ? SetBit(m_reg.mask, 1) : ClearBit(m_reg.mask, 1); }
void Ppu::ShowFgLeft(bool set) { set ? SetBit(m_reg.mask, 2) : ClearBit(m_reg.mask, 2); }
void Ppu::ShowBg(bool set) { set ? SetBit(m_reg.mask, 3) : ClearBit(m_reg.mask, 3); }
void Ppu::ShowFg(bool set) { set ? SetBit(m_reg.mask, 4) : ClearBit(m_reg.mask, 4); }
void Ppu::EmphRed(bool set) { set ? SetBit(m_reg.mask, 5) : ClearBit(m_reg.mask, 5); }
void Ppu::EmphGreen(bool set) { set ? SetBit(m_reg.mask, 6) : ClearBit(m_reg.mask, 6); }
void Ppu::EmphBlue(bool set) { set ? SetBit(m_reg.mask, 7) : ClearBit(m_reg.mask, 7); }

auto Ppu::VBlank() const -> bool { return TestBit(m_reg.status, 7); }
auto Ppu::SpiteZeroHit() const -> bool { return TestBit(m_reg.status, 6); }
auto Ppu::SpriteOverflow() const -> bool { return TestBit(m_reg.status, 5); }

void Ppu::VBlank(bool set) { set ? SetBit(m_reg.status, 7) : ClearBit(m_reg.status, 7); }
void Ppu::SpriteZeroHit(bool set) { set ? SetBit(m_reg.status, 6) : ClearBit(m_reg.status, 6); }
void Ppu::SpriteOverflow(bool set) { set ? SetBit(m_reg.status, 5) : ClearBit(m_reg.status, 5); }

auto Ppu::Latch() -> bool { return m_reg.latch; }
void Ppu::SetLatch() { m_reg.latch = true; }
void Ppu::ToggleLatch() { m_reg.latch = !m_reg.latch; }

void Ppu::RenderCycle() {
  if (m_col == 0) return;  // idle cycle
  else if (m_col <= 256 || (m_col >= 321 && m_col <= 336)) {
    constexpr auto coarse_x_mask = 0b0000'0000'0001'1111;
    constexpr auto coarse_y_mask = 0b0000'0011'1110'0000;
    constexpr auto name_table_mask = 0b0000'1100'0000'0000;
    constexpr auto fine_y_mask = 0b0111'0000'0000'0000;

    auto nt = (m_reg.v & name_table_mask) >> 10;
    auto coarse_x = (m_reg.v & coarse_x_mask) >> 0;
    auto coarse_y = (m_reg.v & coarse_y_mask) >> 5;
    auto fine_y = (m_reg.v & fine_y_mask) >> 12;

    addr_t addr = 0;
    switch (m_col % 8) {
    case 0:
      PrepareShiftRegisters();
      (m_col == 256) ? IncrementVertV() : IncrementHorizV();
      break;
    case 1:
      addr = 0x2000 + (m_reg.v % 0x1000);
      m_reg.bg_next_nt = Read(addr);
      break;
    case 3:
      coarse_x >>= 2;
      coarse_y = (coarse_y >> 2) << 3;
      nt <<= 10;
      addr = 0x23C0 | nt | coarse_y | coarse_x;
      m_reg.bg_next_at = Read(addr);
      if (coarse_x & 0x02) m_reg.bg_next_at >>= 2;
      if (coarse_y & 0x02) m_reg.bg_next_at >>= 4;
      break;
    case 5:
      addr = (0x1000 * BgTable()) + (m_reg.bg_next_nt << 4) + fine_y + 0;
      m_reg.bg_next_id = Read(addr);
      break;
    case 7:
      addr = (0x1000 * BgTable()) + (m_reg.bg_next_nt << 4) + fine_y + 8;
      m_reg.bg_next_id |= Read(addr) << 8;
      break;
    default:
      // each of the above loads takes 2 cycles - we mimic this by skipping loads on even cycles
      break;
    }

    IncrementShiftRegisters();
  } else if (m_col == 257) {
    if (ShowFg() || ShowBg()) {
      auto mask = 0b0000'0100'0001'1111;
      m_reg.v = (m_reg.v & ~mask) | (m_reg.t & mask);
    }
  } else if (m_col <= 320) {
    if (m_row == Row::pre_render && (280 <= m_col && m_col <= 304)) {
      if (ShowFg() || ShowBg()) {
        auto mask = 0b0111'1011'1110'0000;
        m_reg.v = (m_reg.v & ~mask) | (m_reg.t & mask);
      }
    }
    // idle
  } else if (337 <= m_col && m_col <= 340) {
    // unused nametable fetches - let's just pretend nothing happens
  }
}

void Ppu::IncrementHorizV() {
  if (ShowFg() || ShowBg()) {
    if ((m_reg.v & 0x001F) == 0x001F) {
      m_reg.v &= ~0x001F;
      m_reg.v ^= 0x0400;
    } else {
      ++m_reg.v;
    }
  }
}

void Ppu::IncrementVertV() {
  if (ShowFg() || ShowBg()) {
    if ((m_reg.v & 0x7000) != 0x7000) {
      m_reg.v += 0x1000;
    } else {
      m_reg.v &= ~0x7000;
      auto y = (m_reg.v & 0x03E0) >> 5;
      if (y == 29) {
        y = 0;
        m_reg.v ^= 0x0800;
      } else if (y == 31) {
        y = 0;
      } else {
        ++y;
      }
      m_reg.v = (m_reg.v & ~0x03E0) | (y << 5);
    }
  }
}

void Ppu::PrepareShiftRegisters() {
  m_reg.bg_patt_shifter_lo &= 0xFF00;
  m_reg.bg_patt_shifter_hi &= 0xFF00;
  m_reg.bg_attr_shifter_lo &= 0xFF00;
  m_reg.bg_attr_shifter_hi &= 0xFF00;

  auto coarse_x = m_reg.v & 0x001F;
  auto coarse_y = m_reg.v & 0x03E0;
  auto attr_shifter_lo = 0xFF * !!(m_reg.bg_next_at & 1);
  auto attr_shifter_hi = 0xFF * !!(m_reg.bg_next_at & 2);

  m_reg.bg_patt_shifter_lo |= (m_reg.bg_next_id & 0x00FF) >> 0;
  m_reg.bg_patt_shifter_hi |= (m_reg.bg_next_id & 0xFF00) >> 8;
  m_reg.bg_attr_shifter_lo |= attr_shifter_lo;
  m_reg.bg_attr_shifter_hi |= attr_shifter_hi;
}

void Ppu::IncrementShiftRegisters() {
  if (ShowBg()) {
    m_reg.bg_patt_shifter_lo <<= 1;
    m_reg.bg_patt_shifter_hi <<= 1;
    m_reg.bg_attr_shifter_lo <<= 1;
    m_reg.bg_attr_shifter_hi <<= 1;
  }
}

}  // namespace nes