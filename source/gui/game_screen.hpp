#pragma once

#include <wx/dcbuffer.h>
#include <wx/wx.h>

#include "nes/nes.hpp"

namespace gui {

class GameScreen : public wxPanel {
public:
  GameScreen(wxFrame* parent, nes::Console* console)
    : wxPanel{parent, wxID_ANY, wxDefaultPosition, {256, 240}} {
    SetMinSize({256, 240});

    Bind(wxEVT_PAINT, &GameScreen::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, &GameScreen::OnEraseBackground, this);
    Bind(wxEVT_IDLE, &GameScreen::OnIdle, this);
    Bind(wxEVT_KEY_UP, &GameScreen::OnKeyReleased, this);

    m_console = console;
  }

  void SetPixelBuffer(unsigned char* pixel_buffer) { m_pixel_buffer = pixel_buffer; }

  void OnPaint([[maybe_unused]] wxPaintEvent& event) {
    if (m_pixel_buffer == nullptr) return;

    auto [w, h] = GetSize();
    wxBitmap bitmap{wxImage{256, 240, m_pixel_buffer, true}.Scale(w, h)};
    wxBufferedPaintDC dc(this, bitmap);
  }

  void OnEraseBackground(wxEraseEvent&) {}
  void OnIdle(wxIdleEvent& event) {
    Refresh(false);
    wxMilliSleep(16);
    event.RequestMore();
  }

private:
  nes::Console* m_console = nullptr;
  unsigned char* m_pixel_buffer = nullptr;

  void OnKeyReleased(wxKeyEvent& event) {
    event.Skip();
    auto uc = event.GetUnicodeKey();
    if (uc == 'p') m_console->Pause();
  }
};

}  // namespace gui