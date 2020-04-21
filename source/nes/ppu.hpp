#pragma once

#include "nes/common.hpp"
#include "nes/display.hpp"
#include "nes/ppu_bus.hpp"
#include "nes/utility.hpp"

namespace nes {

class Ppu {
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
  Ppu() = default;

  void AttachBus(PpuBus* ppu_bus) { m_bus = AssumeNotNull(ppu_bus); }
  void AttachDisplay(Display* display) { m_display = AssumeNotNull(display); }

  void Step() {
        // TODO: this is just a placeholder draw statement
    if (m_row < Row::max / 2) {
      auto red = Pixel{0xFF'00'00};
      m_display->DrawPixel(m_col, m_row, red);
    } else {
      auto blue = Pixel{0x00'00'FF};
      m_display->DrawPixel(m_col, m_row, blue);
    }

    // The PPU skips the point (340, 261) on odd frames. This is equivalent to skipping the idle
    // step at (0, 0), and letting scanline 261 be a full render line.
    if (m_row == 0 && m_col == 0) { m_col = m_frame_odd ? 1 : 0; }

    if (m_row < Row::screen_height || m_row == Row::pre_render) { RenderCycle(); }

    if ((m_row == Row::vblank_set) && (m_col == Col::vblank_set)) {
      // TODO: set v-blank flag
    }

    if (m_row == Row::vblank_clear && m_col == Col::vblank_clear) {
      // TODO: clear v-blank flag
    }

    MoveDot();
  }

private:
  uint m_row = 261;
  uint m_col = 0;  // the current scanline
  bool m_frame_odd = false;
  PpuBus* m_bus = nullptr;
  Display* m_display = nullptr;

  void MoveDot() {
    if (++m_col > Col::max) {
      m_col = 0;
      ++m_row;
      if (++m_row > Row::max) {
        m_row = 0;
        m_frame_odd = !m_frame_odd;
      }
    }
  }

  void RenderCycle() {
    if (m_col == 0) return;  // idle cycle

    if (m_col <= 256) {
      switch ((m_col - 1) & 0x07) {
      case 0: break;
      case 1:
        // TODO: load name table byte
        break;
      case 2: break;
      case 3:
        // TODO: load attribute table byte
      case 4: break;
      case 5:
        // TODO: load pattern table low tile
      case 6: break;
      case 7:
        // TODO: load pattern table high tile & increment horizontal part of v register
      default: break;
      }
    } else if (m_col <= 320) {
    }
  }

  void PostRenderCycle() {}

  void PreRenderCycle() {}
};

}  // namespace nes