#pragma once

#include <array>

#include "nes/bus.hpp"
#include "nes/cartridge.hpp"
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
  using NameTable = std::array<byte_t, 0x0400>;

  struct Registers {
    // memory mapped registers
    byte_t control = 0;
    byte_t mask = 0;
    byte_t status = 0;
    byte_t data = 0;
    byte_t oam_address = 0;
    byte_t oam_data = 0;
    byte_t oam_dma = 0;

    // see https://wiki.nesdev.com/w/index.php/PPU_scrolling for details on these registers
    addr_t v = 0;
    addr_t t = 0;
    byte_t x = 0;
    bool latch = true;

    // background rendering bits - thanks to javidx9 for clarifying how this works!
    // source: https://www.youtube.com/watch?v=-THeUXqR3zY
    byte_t bg_next_nt = 0;
    byte_t bg_next_at = 0;
    addr_t bg_next_id = 0;
    addr_t bg_patt_shifter_lo = 0;
    addr_t bg_patt_shifter_hi = 0;
    addr_t bg_attr_shifter_lo = 0;
    addr_t bg_attr_shifter_hi = 0;
  };

  struct Sprite {
    byte_t x;
    byte_t y;
    byte_t tile;
    byte_t attr;
  };

  void Reset();

  void AttachBus(Bus* bus);
  void AttachCartridge(Cartridge* cartridge);
  void AttachDisplay(Display* display);

  void Step();

private:
  Bus* m_bus = nullptr;
  Display* m_display = nullptr;
  Cartridge* m_cartridge = nullptr;

  uint m_row = 261;  // often called scanlines
  uint m_col = 0;    // often called cycles or dots
  bool m_frame_odd = false;

  Registers m_reg = {};
  std::array<PatternTable, 2> m_pattern_tables = {};
  std::array<NameTable, 4> m_name_tables = {};  // name tables include attribute tables
  std::array<byte_t, 0x20> m_palette_table = {};
  std::array<Sprite, 64> m_sprites = {};  // also called OAM - Object Attribute Memory

  void DrawPixel();

  auto Read(addr_t addr) const -> byte_t;
  void Write(addr_t addr, byte_t value);

  // control register read/write
  auto NtBaseX() const -> bool;
  auto NtBaseY() const -> bool;
  auto IncModeY() const -> bool;
  auto SpriteTable() const -> bool;
  auto BgTable() const -> bool;
  auto BigSprites() const -> bool;
  auto ExtWrite() const -> bool;
  auto GenNmi() const -> bool;

  void NtBaseX(bool set);
  void NtBaseY(bool set);
  void IncModeY(bool set);
  void SpriteTable(bool set);
  void BgTable(bool set);
  void BigSprites(bool set);
  void ExtWrite(bool set);
  void GenNmi(bool set);

  // mask register read/write
  auto Grayscale() const -> bool;
  auto ShowBgLeft() const -> bool;
  auto ShowFgLeft() const -> bool;
  auto ShowBg() const -> bool;
  auto ShowFg() const -> bool;
  auto EmphRed() const -> bool;
  auto EmphGreen() const -> bool;
  auto EmphBlue() const -> bool;

  void Grayscale(bool set);
  void ShowBgLeft(bool set);
  void ShowFgLeft(bool set);
  void ShowBg(bool set);
  void ShowFg(bool set);
  void EmphRed(bool set);
  void EmphGreen(bool set);
  void EmphBlue(bool set);

  // status register read/write
  auto VBlank() const -> bool;
  auto SpiteZeroHit() const -> bool;
  auto SpriteOverflow() const -> bool;

  void VBlank(bool set);
  void SpriteZeroHit(bool set);
  void SpriteOverflow(bool set);

  auto Latch() -> bool;
  void SetLatch();
  void ToggleLatch();

  void RenderCycle();

  void IncrementHorizV();
  void IncrementVertV();
  void PrepareShiftRegisters();
  void IncrementShiftRegisters();
};

}  // namespace nes