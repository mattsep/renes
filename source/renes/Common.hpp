#pragma once

#include <cstdint>

namespace renes {

using byte = std::uint8_t;
using addr = std::uint16_t;
using zstr = const char*;

// forward declarations
class Bus;
class Cartridge;
class Cpu;
class Mapper;
class Nes;

}