#pragma once

#include <wx/dcbuffer.h>
#include <wx/wx.h>

namespace gui {

class GameScreen : public wxPanel {
  // wxDECLARE_EVENT_TABLE();

public:
  GameScreen(wxFrame* parent) : wxPanel{parent, wxID_ANY, wxDefaultPosition, {256, 240}} {
    SetMinSize({256, 240});
    Bind(wxEVT_PAINT, &GameScreen::OnPaint, this);
    Bind(wxEVT_ERASE_BACKGROUND, &GameScreen::OnEraseBackground, this);
    Bind(wxEVT_IDLE, &GameScreen::OnIdle, this);
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
  unsigned char* m_pixel_buffer;
};

// // clang-format off
// wxBEGIN_EVENT_TABLE(GameScreen, wxPanel)
//   EVT_PAINT(GameScreen::OnPaint)
//   EVT_ERASE_BACKGROUND(GameScreen::OnEraseBackground)
//   EVT_IDLE(GameScreen::OnIdle)
// wxEND_EVENT_TABLE()
// clang-format on

}  // namespace gui