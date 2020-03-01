#include <iostream>

#include "renes/Nes.hpp"

int main()
{
  using namespace renes;

  try {
    auto nes = Nes{};
  } catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << '\n';
  }

  return 0;
}