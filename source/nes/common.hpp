#pragma once

#include <cstdint>
#include <string>

#include "nes/logger.hpp"

namespace nes {

using byte_t = std::uint8_t;
using addr_t = std::uint16_t;

using uint = unsigned int;
using std::size_t;
using std::string;

// forward declarations
class Nes;
class Cpu;
class Ppu;
class MainBus;
class PpuBus;
class Cartridge;
class Mapper;

}  // namespace nes