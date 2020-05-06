#pragma once

#include <wx/wx.h>

#include "gui/game_screen.hpp"
#include "nes/nes.hpp"

namespace gui {

class MainWindow : public wxFrame {
  wxDECLARE_EVENT_TABLE();

public:
  MainWindow(nes::Console* console) : wxFrame{nullptr, wxID_ANY, "ReNES Emulator"} {
    m_console = console;

    // yes, the const_cast makes me uncomfortable too... unfortunately, the wxImage constructor
    // (used later) requires a pointer to non-const unsigned chars, even though it should only need
    // read access to the data
    using Display = std::remove_cv_t<std::remove_reference_t<decltype(m_console->GetDisplay())>>;
    auto* pixel_buffer = const_cast<Display&>(m_console->GetDisplay()).GetRawPixelBuffer();

    m_game_screen = new GameScreen(this);
    m_game_screen->SetMinSize(m_game_screen->GetSize());
    m_game_screen->SetPixelBuffer(pixel_buffer);

    auto sizer = new wxFlexGridSizer{2, 2, 0, 0};
    sizer->Add(m_game_screen, wxSizerFlags().Shaped().Border(wxALL, 4));

    m_menu_bar = new wxMenuBar();
    MakeFileMenu();
    MakeToolsMenu();

    SetMenuBar(m_menu_bar);

    sizer->Fit(this);
    SetSizer(sizer);

    Layout();
    Center(wxBOTH);
  }

private:
  wxMenuBar* m_menu_bar = nullptr;
  wxMenu* m_file_menu = nullptr;
  wxMenu* m_tools_menu = nullptr;
  GameScreen* m_game_screen = nullptr;
  nes::Console* m_console = nullptr;
  wxListBox* m_register_box = nullptr;

  void MakeFileMenu() {
    m_file_menu = new wxMenu();

    auto file_open = new wxMenuItem(m_file_menu, wxID_ANY, "Open...\tCTRL+O");
    m_file_menu->Append(file_open);

    // auto file_save = new wxMenuItem(m_file_menu, wxID_ANY, "Save\tCTRL+S");
    // m_file_menu->Append(file_save);

    m_menu_bar->Append(m_file_menu, "File");

    m_file_menu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnFileOpen),
                      this, file_open->GetId());
  }

  void MakeToolsMenu() {
    m_tools_menu = new wxMenu();

    auto tools_cpu_registers = new wxMenuItem(m_tools_menu, wxID_ANY, "View CPU Registers");
    m_tools_menu->Append(tools_cpu_registers);

    auto tools_cpu_instructions = new wxMenuItem(m_tools_menu, wxID_ANY, "View CPU Instructions");
    m_tools_menu->Append(tools_cpu_instructions);

    m_menu_bar->Append(m_tools_menu, "Tools");

    m_tools_menu->Bind(wxEVT_COMMAND_MENU_SELECTED,
                       wxCommandEventHandler(MainWindow::OnToolsViewCpuRegisters), this,
                       tools_cpu_registers->GetId());
    m_tools_menu->Bind(wxEVT_COMMAND_MENU_SELECTED,
                       wxCommandEventHandler(MainWindow::OnToolsViewCpuInstructions), this,
                       tools_cpu_instructions->GetId());
  }

  void OnFileOpen(wxCommandEvent& event) {
    m_console->Pause();

    wxFileDialog file_browser{this, "NES files (*.nes)|*.nes"};
    if (file_browser.ShowModal() == wxID_OK) {
      auto file = file_browser.GetPath().ToStdString();
      LOG_DEBUG("[GUI] User selected '" + file + '\'');
      m_console->Load(file);
    } else {
      m_console->Unpause();
    }

    event.Skip();
  }

  void OnToolsViewCpuRegisters([[maybe_unused]] wxCommandEvent& event) {
    if (m_register_box == nullptr) {
      m_register_box = new wxListBox{this, wxID_ANY, wxDefaultPosition, m_game_screen->GetSize()};
      GetSizer()->Add(m_register_box, wxSizerFlags().Border(wxALL, 4));
      GetSizer()->Fit(this);
      Layout();

      auto& reg = m_console->GetCpu().GetRegisters();
      m_register_box->Append("Program Counter: " + nes::Hexify(reg.pc));
      m_register_box->Append("Accumulator: " + nes::Hexify(reg.a));
      m_register_box->Append("X Index: " + nes::Hexify(reg.x));
      m_register_box->Append("Y Index: " + nes::Hexify(reg.y));
      m_register_box->Append("Stack Pointer: " + nes::Hexify(reg.s));
      m_register_box->Append("CPU Status: " + nes::Hexify(reg.p));
    }
  }
  
  void OnToolsViewCpuInstructions([[maybe_unused]] wxCommandEvent& event) {}

  void OnIdle([[maybe_unused]] wxIdleEvent& event) {
    LOG_DEBUG("[GUI] Updating register view!");
    if (m_register_box) {
      auto& reg = m_console->GetCpu().GetRegisters();
      m_register_box->SetString(0, "Program Counter: " + nes::Hexify(reg.pc));
      m_register_box->SetString(1, "Accumulator: " + nes::Hexify(reg.a));
      m_register_box->SetString(2, "X Index: " + nes::Hexify(reg.x));
      m_register_box->SetString(3, "Y Index: " + nes::Hexify(reg.y));
      m_register_box->SetString(4, "Stack Pointer: " + nes::Hexify(reg.s));
      m_register_box->SetString(5, "CPU Status: " + nes::Hexify(reg.p));
    }
    // event.RequestMore();
  }

  void OnKeyReleased(wxKeyEvent& event) {
    LOG_INFO("In an event!");
    auto uc = event.GetUnicodeKey();
    if (uc != WXK_NONE) {
      LOG_INFO("[GUI] Pausing console");
      if (uc == 'P') m_console->TogglePause();
    } else {
      LOG_INFO(uc);
    }
  }
};

// clang-format off
wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
  // EVT_IDLE(MainWindow::OnIdle)
  EVT_KEY_UP(MainWindow::OnKeyReleased)
wxEND_EVENT_TABLE()
// clang-format on

}  // namespace gui