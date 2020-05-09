#include "nes/mappers.hpp"

namespace nes {

auto CreateMapper(uint mapper) -> std::unique_ptr<Mapper> {
  switch (mapper) {
  case 0: return std::make_unique<Mapper_000>();
  default: return nullptr;
  }
}

}