#pragma once

#include <wx/wx.h>

#include "gui/main_window.hpp"

namespace gui {

template <class Console>
class Application : public wxApp {
public:
  Application() = default;
  Application(Console* console) : m_console{console} {}

private:
  Console* m_console = nullptr;
  MainWindow* m_main_window = nullptr;

  auto OnInit() -> bool override {
    wxInitAllImageHandlers();

    // yes, the const_cast makes me uncomfortable too... unfortunately, the wxImage constructor
    // (used later) requires a pointer to non-const unsigned chars, even though it should only need
    // read access to the data
    using Display = std::remove_cv_t<std::remove_reference_t<decltype(m_console->GetDisplay())>>;
    auto* pixel_buffer = const_cast<Display&>(m_console->GetDisplay()).GetRawPixelBuffer();

    m_main_window = new MainWindow{pixel_buffer};
    m_main_window->Show();

    return true;
  }

  auto OnExit() -> int override {
    m_console->PowerOff();
    return 0;
  }
};

}  // namespace gui