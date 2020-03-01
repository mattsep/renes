#pragma once

#include <memory>

#include "renes/Bus.hpp"
#include "renes/Cpu.hpp"

namespace renes {

class Nes {
public:
  void LoadCartridge();

private:
  Bus bus_ = {};
  Cpu cpu_ = {bus_};
};

}  // namespace renes