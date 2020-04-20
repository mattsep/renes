#pragma once

#include <wx/wx.h>

#include "gui/game_screen.hpp"

namespace gui {

class MainWindow : public wxFrame {
public:
  MainWindow(unsigned char* pixel_buffer) : wxFrame{nullptr, wxID_ANY, "ReNES Emulator"} {
    m_menu_bar = new wxMenuBar();

    m_game_screen = new GameScreen(this);
    m_game_screen->SetMinSize(m_game_screen->GetSize());
    m_game_screen->SetPixelBuffer(pixel_buffer);

    auto sizer = new wxFlexGridSizer{2, 2, 0, 0};
    sizer->Add(m_game_screen, wxALL | wxALIGN_CENTER | wxFIXED_MINSIZE | wxSHAPED);

    MakeFileMenu();
    MakeToolsMenu();

    SetMenuBar(m_menu_bar);
    SetSizer(sizer);
    Layout();
    Center(wxBOTH);
  }

private:
  wxMenuBar* m_menu_bar;
  wxMenu* m_file_menu;
  wxMenu* m_tools_menu;
  GameScreen* m_game_screen;
  wxListBox* m_list_box;

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
    wxFileDialog file_browser{this, "NES files (*.nes)|*.nes"};
    if (file_browser.ShowModal() == wxOK) {}
    event.Skip();
  }

  void OnToolsViewCpuRegisters(wxCommandEvent& event) {}
  void OnToolsViewCpuInstructions(wxCommandEvent& event) {}
};

}  // namespace gui