#pragma once

#include "nes/common.hpp"
#include "nes/display.hpp"
#include "nes/ppu_bus.hpp"
#include "nes/utility.hpp"

namespace nes {

class Ppu {
public:
  Ppu() = default;

  void AttachBus(PpuBus* ppu_bus) { m_bus = AssumeNotNull(ppu_bus); }
  void AttachDisplay(Display* display) { m_display = AssumeNotNull(display); }

  void Step() {}

private:
  PpuBus* m_bus = nullptr;
  Display* m_display = nullptr;
};

}  // namespace nes