#pragma once

#include <cassert>

#include "nes/common.hpp"

namespace nes {

class Pixel {
public:
  constexpr Pixel() = default;

  constexpr Pixel(std::uint32_t rgb)
    : m_rgb{static_cast<byte_t>((rgb >> 16) & 0xFF), static_cast<byte_t>((rgb >> 8) & 0xFF),
            static_cast<byte_t>((rgb >> 0) & 0xFF)} {}

  constexpr Pixel(byte_t red, byte_t blue, byte_t green) : m_rgb{red, blue, green} {}

  constexpr auto Red() const -> byte_t { return m_rgb[0]; }
  constexpr auto Blue() const -> byte_t { return m_rgb[1]; }
  constexpr auto Green() const -> byte_t { return m_rgb[2]; }

  auto Red() -> byte_t& { return m_rgb[0]; }
  auto Blue() -> byte_t& { return m_rgb[1]; }
  auto Green() -> byte_t& { return m_rgb[2]; }

private:
  std::array<byte_t, 3> m_rgb = {};
};

class Display {
public:
  static constexpr auto Size() { return std::array{m_width, m_height}; }
  static constexpr auto Width() { return m_width; }
  static constexpr auto Height() { return m_height; }

  void DrawPixel(size_t x, size_t y, Pixel pixel);
  auto ReadPixel(size_t x, size_t y) -> Pixel;
  auto GetRawPixelBuffer() -> byte_t*;

private:
  static constexpr size_t m_width = 256;
  static constexpr size_t m_height = 240;
  std::array<Pixel, m_width* m_height> m_pixels = {};

  auto GetIndex(size_t x, size_t y) const -> size_t;
};

}  // namespace nes