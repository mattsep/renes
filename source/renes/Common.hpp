#pragma once

#include <cstdint>
#include <string>

namespace renes {

using byte_t = std::uint8_t;
using addr_t = std::uint16_t;

using std::size_t;
using std::string;

// forward declarations
class Bus;
class Cartridge;
class Cpu;
class Mapper;
class Nes;
class Ppu;

}  // namespace renes