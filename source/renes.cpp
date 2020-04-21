#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include "gui/gui.hpp"
#include "nes/nes.hpp"

wxIMPLEMENT_APP_NO_MAIN(gui::Application<nes::Console>);

void RunGui(int argc, char* argv[], nes::Console* console);
void RunNes(int argc, char* argv[], nes::Console* console);

int main(int argc, char* argv[]) {
  LOG_LEVEL(Debug);
  LOG_INFO("Starting ReNES");

  auto console = nes::Console{};

  auto gui_thread = std::thread{RunGui, argc, argv, &console};
  auto nes_thread = std::thread{RunNes, argc, argv, &console};

  gui_thread.join();
  nes_thread.join();

  return 0;
}

void RunGui(int argc, char* argv[], nes::Console* console) {
  auto app = new gui::Application{console};
  wxApp::SetInstance(app);
  wxEntry(argc, argv);
}

void RunNes(int argc, char* argv[], nes::Console* console) {
  if (argc > 1) {
    std::vector<std::string> args(argv + 1, argv + argc);

    // TODO: need to parse command line arguments correctly
    LOG_DEBUG("Found " + std::to_string(argc) + " command line arguments:");
    for (auto arg : args) LOG_DEBUG(arg + ", ");

    auto rom = std::string{argv[1]};
    console->Load(rom);
  }

  try {
    console->Run();
  } catch (std::exception& e) { std::cerr << e.what() << std::endl; }
}
