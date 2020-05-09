#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include "nes/common.hpp"
#include "nes/mappers/mapper.hpp"
#include "nes/mappers/mapper_000.hpp"

namespace nes {

auto CreateMapper(uint mapper) -> std::unique_ptr<Mapper>;

}