#pragma once

#include <array>

#include "nes/common.hpp"
#include "nes/display.hpp"
#include "nes/pallete.hpp"
#include "nes/utility.hpp"

namespace nes {

class Ppu {
  friend class Bus;

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
  using PatternTable = std::array<byte_t, 0x1000>;
  using NameTable = std::array<byte_t, 0x03C0>;
  using AttributeTable = std::array<byte_t, 0x0040>;

  struct Registers {
    byte_t control;
    byte_t mask;
    byte_t status;
    byte_t scroll_x;
    byte_t scroll_y;
    addr_t address;
    byte_t data;
    byte_t oam_address;
    byte_t oam_data;
    byte_t oam_dma;
  };

  struct Sprite {
    byte_t x;
    byte_t y;
    byte_t tile;
    byte_t attr;
  };

  void AttachDisplay(Display* display);
  void Step();

private:
  Display* m_display = nullptr;
  uint m_row = 261;
  uint m_col = 0;
  bool m_frame_odd = false;
  Registers m_reg = {};
  std::array<PatternTable, 2> m_pattern_tables;
  std::array<NameTable, 4> m_name_tables;
  std::array<AttributeTable, 4> m_attribute_tables;  // technically part of the name tables
  std::array<Sprite, 64> m_sprites;                  // also called OAM - Object Attribute Memory

  void MoveDot();

  auto Read(addr_t addr) -> byte_t;
  void Write(addr_t addr, byte_t value);

  auto VBlank() const -> bool;
  auto SpiteZeroHit() const -> bool;
  auto SpriteOverflow() const -> bool;

  void VBlank(bool set);
  void SpriteZeroHit(bool set);
  void SpriteOverflow(bool set);

  void RenderCycle();
  void PostRenderCycle();
  void PreRenderCycle();
};

}  // namespace nes