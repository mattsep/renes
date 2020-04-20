#pragma once

#include <wx/dcbuffer.h>
#include <wx/wx.h>

namespace gui {

class GameScreen : public wxPanel {
  wxDECLARE_EVENT_TABLE();

public:
  GameScreen(wxFrame* parent) : wxPanel{parent, wxID_ANY, wxDefaultPosition, {256, 240}} {}

  void SetPixelBuffer(unsigned char* pixel_buffer) { m_pixels = pixel_buffer; }

  void OnPaint(wxPaintEvent& event) {
    if (m_pixels == nullptr) return;

    auto [w, h] = GetSize();
    wxBitmap bitmap{wxImage{256, 240, m_pixels, true}.Scale(w, h)};
    wxBufferedPaintDC dc(this, bitmap);
  }

  void OnEraseBackground(wxEraseEvent&) {}
  void OnIdle(wxIdleEvent& event) {
    Refresh(false);
    wxMilliSleep(16);
  }

private:
  unsigned char* m_pixels;
};

// clang-format off
wxBEGIN_EVENT_TABLE(GameScreen, wxPanel)
  EVT_PAINT(GameScreen::OnPaint)
  EVT_ERASE_BACKGROUND(GameScreen::OnEraseBackground)
  EVT_IDLE(GameScreen::OnIdle)
wxEND_EVENT_TABLE()
// clang-format on

}  // namespace gui