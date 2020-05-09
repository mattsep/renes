#include "nes/display.hpp"

namespace nes {

void Display::DrawPixel(size_t x, size_t y, Pixel pixel) {
  if (auto index = GetIndex(x, y); index < m_pixels.size()) { m_pixels[index] = pixel; }
}

auto Display::ReadPixel(size_t x, size_t y) -> Pixel {
  if (auto index = GetIndex(x, y); index < m_pixels.size()) {
    return m_pixels[GetIndex(x, y)];
  } else {
    return {};
  }
}

auto Display::GetRawPixelBuffer() -> byte_t* { return reinterpret_cast<byte_t*>(m_pixels.data()); }

auto Display::GetIndex(size_t x, size_t y) const -> size_t {
  if (x < m_width && y < m_height) {
    return x + m_width * y;
  } else {
    return m_pixels.size();
  }
}

}  // namespace nes