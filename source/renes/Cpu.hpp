#pragma once

#include "renes/Common.hpp"

namespace renes {

class Cpu {
public:
  enum class State {
    Unknown,
    Read,
    Write,
  };

  Cpu(Bus& bus) : bus_{bus} {}

private:
  Bus& bus_;
};

}  // namespace renes