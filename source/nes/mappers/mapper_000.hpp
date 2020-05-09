#pragma once

#include "nes/common.hpp"
#include "nes/mappers/mapper.hpp"

namespace nes {

class Mapper_000 : public Mapper {
public:
  ~Mapper_000() = default;

  auto CpuRead(addr_t addr) -> byte_t;
  auto CpuWrite(addr_t, byte_t) -> byte_t;

  auto PpuRead(addr_t addr) -> byte_t;
  auto PpuWrite(addr_t, byte_t) -> byte_t;
};

}  // namespace nes