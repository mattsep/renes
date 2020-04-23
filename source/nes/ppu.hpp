#pragma once

#include <array>

#include "nes/common.hpp"
#include "nes/display.hpp"
#include "nes/ppu_bus.hpp"
#include "nes/utility.hpp"

namespace nes {

class Ppu {
  friend class MainBus;

  struct Row {
    static constexpr uint screen_height = Display::Height();
    static constexpr uint post_render = 240;
    static constexpr uint vblank_set = 241;
    static constexpr uint vblank_clear = 261;
    static constexpr uint pre_render = 261;
    static constexpr uint max = 261;
  };

  struct Col {
    static constexpr uint screen_width = Display::Width();
    static constexpr uint sprite_prefetch_area = 321;
    static constexpr uint tile_prefetch_area = 337;
    static constexpr uint vblank_set = 1;
    static constexpr uint vblank_clear = 1;
    static constexpr uint max = 340;
  };

public:
  struct Registers {
    byte_t control;
    byte_t mask;
    byte_t status;
    byte_t scroll;
    byte_t address;
    byte_t data;
    byte_t oam_address;
    byte_t oam_data;
    byte_t oam_dma;
  };

  Ppu() = default;

  void AttachBus(PpuBus* ppu_bus) { m_bus = AssumeNotNull(ppu_bus); }
  void AttachDisplay(Display* display) { m_display = AssumeNotNull(display); }

  void Step() {
    LOG_TRACE("\tcurrent pixel: " + std::to_string(m_col) + ", " + std::to_string(m_row));

    // TODO: this is just a placeholder draw statement
    if (m_row < Row::max / 2) {
      auto purple = Pixel{0xFF'00'FF};
      m_display->DrawPixel(m_col, m_row, purple);
    } else {
      auto teal = Pixel{0x00'FF'FF};
      m_display->DrawPixel(m_col, m_row, teal);
    }

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

private:
  Registers m_reg = {};
  PpuBus* m_bus = nullptr;
  Display* m_display = nullptr;
  uint m_row = 261;
  uint m_col = 0;
  // std::array<byte_t, 256> m_oam;
  bool m_frame_odd = false;

  void MoveDot() {
    if (++m_col > Col::max) {
      m_col = 0;
      if (++m_row > Row::max) {
        m_row = 0;
        m_frame_odd = !m_frame_odd;
      }
    }
  }

  auto VBlank() -> bool { return m_reg.status & (1u << 7); }
  void VBlank(bool set) {
    auto x = static_cast<unsigned>(set);
    m_reg.status = static_cast<byte_t>((m_reg.status & ~(1u << 7)) | (x << 7));
  }

  void RenderCycle() {
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

  void PostRenderCycle() {}

  void PreRenderCycle() {}
};

}  // namespace nes