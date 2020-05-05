#pragma once

#include <array>

#include "nes/common.hpp"
#include "nes/main_bus.hpp"
#include "nes/utility.hpp"

namespace nes {

class PpuBus {
public:
private:
  [[maybe_unused]] std::array<byte_t, 0x2000> m_pattern_tables;
  [[maybe_unused]] std::array<byte_t, 0x1000> m_name_tables;
};

}  // namespace nes