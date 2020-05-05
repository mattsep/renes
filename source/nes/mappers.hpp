#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include "nes/common.hpp"
#include "nes/mappers/mapper.hpp"
#include "nes/mappers/mapper_000.hpp"

namespace nes {

auto CreateMapper(uint mapper) -> std::unique_ptr<Mapper> {
  switch (mapper) {
  case 0: return std::make_unique<Mapper_000>();
  default: return nullptr;
  }
}

}