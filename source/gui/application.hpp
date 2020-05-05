#pragma once

#include <wx/wx.h>

#include "gui/main_window.hpp"
#include "nes/nes.hpp"

namespace gui {

class Application : public wxApp {
public:
  Application() = default;
  Application(nes::Console* console) : m_console{console} {}

private:
  nes::Console* m_console = nullptr;
  MainWindow* m_main_window = nullptr;

  auto OnInit() -> bool override {
    wxInitAllImageHandlers();

    m_main_window = new MainWindow{m_console};
    m_main_window->Show();

    return true;
  }

  auto OnExit() -> int override {
    m_console->PowerOff();
    return 0;
  }
};

}  // namespace gui