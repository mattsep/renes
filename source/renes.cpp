#include <future>
#include <iostream>
#include <thread>

#include "gui/gui.hpp"
#include "nes/nes.hpp"

wxIMPLEMENT_APP_NO_MAIN(gui::Application<nes::Console>);

void RunGui(int argc, char* argv[], bool* running, nes::Console* console);
void RunNes(int argc, char* argv[], bool* running, nes::Console* console);

int main(int argc, char* argv[]) {
  bool running = true;
  auto console = nes::Console{};

  auto gui_thread = std::thread{RunGui, argc, argv, &running, &console};
  auto nes_thread = std::thread{RunNes, argc, argv, &running, &console};

  gui_thread.join();
  nes_thread.join();

  return 0;
}

void RunGui(int argc, char* argv[], bool* running, nes::Console* console) {
  auto app = new gui::Application{console};
  wxApp::SetInstance(app);
  wxEntry(argc, argv);
  *running = false;
}

void RunNes(int argc, char* argv[], bool* running, nes::Console* console) {
  try {
    console->Run();
  } catch (std::exception& e) { std::cerr << e.what() << std::endl; }
}
