#pragma once

#include <cassert>

#include "nes/common.hpp"

namespace nes {

class Pixel {
public:
  constexpr Pixel() = default;

  constexpr Pixel(std::uint32_t rgb)
    : m_rgb{static_cast<byte_t>((rgb >> 020) & 0xFF), static_cast<byte_t>((rgb >> 010) & 0xFF),
            static_cast<byte_t>((rgb >> 000) & 0xFF)} {}

  constexpr Pixel(byte_t red, byte_t blue, byte_t green) : m_rgb{red, blue, green} {}

  constexpr auto Red() const { return m_rgb[0]; }
  constexpr auto Blue() const { return m_rgb[1]; }
  constexpr auto Green() const { return m_rgb[2]; }

  auto& Red() { return m_rgb[0]; }
  auto& Blue() { return m_rgb[1]; }
  auto& Green() { return m_rgb[2]; }

private:
  std::array<byte_t, 3> m_rgb = {};
};

class Display {
public:
  constexpr auto Size() { return std::array{m_width, m_height}; }
  constexpr auto Width() { return m_width; }
  constexpr auto Height() { return m_height; }

  auto const& GetPixelBuffer() const { return m_pixels; }

  auto operator()(int x, int y) -> Pixel&{
    assert(x >= 0 && x < m_width);
    assert(y >= 0 && y < m_height);

    auto index = x + m_width * y;
    return m_pixels[index];
  }

private:
  static constexpr int m_width = 256;
  static constexpr int m_height = 240;
  std::array<Pixel, m_width* m_height> m_pixels = {};
};

}  // namespace nes