#pragma once

#include <wx/wx.h>

#include "gui/main_window.hpp"

namespace gui {

template <class Console>
class Application : public wxApp {
public:
  Application() = default;
  Application(Console const* console) : m_console{console} {}

private:
  Console const* m_console = nullptr;
  MainWindow* m_main_window = nullptr;

  auto OnInit() -> bool override {
    using byte = unsigned char;

    wxInitAllImageHandlers();

    // yes, the const_cast makes me uncomfortable too... unfortunately, the wxImage constructor
    // (used later) requires a pointer to non-const unsigned chars, even though it should only need
    // read access to the data
    auto& pixel_buffer = m_console->GetDisplay().GetPixelBuffer();
    auto* raw_pixels = const_cast<byte*>(reinterpret_cast<byte const*>(pixel_buffer.data()));

    m_main_window = new MainWindow{raw_pixels};
    m_main_window->Show();

    return true;
  }
};

}  // namespace gui