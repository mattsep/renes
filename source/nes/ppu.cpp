#include "nes/ppu.hpp"

namespace nes {

// ----------------------------------------------
// Public member function definitions
// ----------------------------------------------

void Ppu::AttachDisplay(Display* display) { m_display = AssumeNotNull(display); }

void Ppu::Step() {
  LOG_TRACE("[PPU] current pixel: " + std::to_string(m_col) + ", " + std::to_string(m_row));

  // TODO: this is just a placeholder draw statement
  auto color = Pixel{};
  if (m_row > Row::screen_height / 3 && m_row < 2 * Row::screen_height / 3 &&
      m_col > Col::screen_width / 3 && m_col < 2 * Col::screen_width / 3 && m_frame_odd) {
    color = Pixel{0xFF'00'FF};
  } else {
    color = Pixel{0x00'FF'FF};
  }
  m_display->DrawPixel(m_col, m_row, color);

  // The PPU skips the point (340, 261) on odd frames. This is equivalent to skipping the idle
  // step at (0, 0), and letting scanline 261 be a full render line.
  if (m_row == 0 && m_col == 0) { m_col = m_frame_odd ? 1 : 0; }

  if (m_row < Row::screen_height || m_row == Row::pre_render) { RenderCycle(); }

  if ((m_row == Row::vblank_set) && (m_col == Col::vblank_set)) {
    // TODO: set v-blank flag
    VBlank(true);
  }

  if (m_row == Row::vblank_clear && m_col == Col::vblank_clear) {
    // TODO: clear status flags
    VBlank(false);
  }

  MoveDot();
}

// ----------------------------------------------
// Private member function definitions
// ----------------------------------------------

  void Ppu::MoveDot() {
    if (++m_col > Col::max) {
      m_col = 0;
      if (++m_row > Row::max) {
        m_row = 0;
        m_frame_odd = !m_frame_odd;
      }
    }
  }

  auto Ppu::Read(addr_t addr) -> byte_t { return void(addr), 0; }
  void Ppu::Write(addr_t addr, byte_t value) { addr, value, void(0); }

  auto Ppu::VBlank() const -> bool { return TestBit(m_reg.status, 7); }
  auto Ppu::SpiteZeroHit() const -> bool { return TestBit(m_reg.status, 6); }
  auto Ppu::SpriteOverflow() const -> bool { return TestBit(m_reg.status, 5); }

  void Ppu::VBlank(bool set) { set ? SetBit(m_reg.status, 7) : ClearBit(m_reg.status, 7); }
  void Ppu::SpriteZeroHit(bool set) { set ? SetBit(m_reg.status, 6) : ClearBit(m_reg.status, 6); }
  void Ppu::SpriteOverflow(bool set) { set ? SetBit(m_reg.status, 5) : ClearBit(m_reg.status, 5); }

  void Ppu::RenderCycle() {
    if (m_col == 0) return;  // idle cycle

    if (m_col <= 256) {
      switch ((m_col - 1) & 0x07) {
      case 1:
        // TODO: load name table byte
        break;
      case 3:
        // TODO: load attribute table byte
        break;
      case 5:
        // TODO: load pattern table low tile
        break;
      case 7:
        // TODO: load pattern table high tile & increment horizontal part of v register
        break;
      default:
        // each of the above loads takes 2 cycles - we mimic this by skipping loads on even cycles
        break;
      }
    } else if (m_col <= 320) {
    }
  }

  void Ppu::PostRenderCycle() {}
  void Ppu::PreRenderCycle() {}

}  // namespace nes