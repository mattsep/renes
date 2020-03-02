#pragma once

#include "renes/Common.hpp"

namespace renes {

class Bus {
public:

  u8 Read(u16 addr);
  void Write(u16 addr, u8 value);

private:

};

}  // namespace renes