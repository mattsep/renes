#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include "gui/gui.hpp"
#include "nes/nes.hpp"

wxIMPLEMENT_APP_NO_MAIN(gui::Application<nes::Console>);

struct Options {
  bool print_help = false;
  nes::LogLevel log_level = nes::LogLevel::Info;
  std::string rom_file = "";
};

void PrintHelp();
Options ParseArgs(int argc, char* argv[]);

void RunGui(int argc, char* argv[], nes::Console* console);
void RunNes(Options const& options, nes::Console* console);

int main(int argc, char* argv[]) {
  auto options = ParseArgs(argc, argv);
  if (options.print_help) {
    PrintHelp();
    return 0;
  }

  auto console = nes::Console{};

  auto gui_thread = std::thread{RunGui, argc, argv, &console};
  auto nes_thread = std::thread{RunNes, options, &console};

  gui_thread.join();
  nes_thread.join();

  return 0;
}

void RunGui(int argc, char* argv[], nes::Console* console) {
  auto app = new gui::Application{console};
  wxApp::SetInstance(app);
  wxEntry(argc, argv);
}

void RunNes(Options const& options, nes::Console* console) {
  LOG_INFO("Starting ReNES");

  try {
    console->Load(options.rom_file);
    console->Run();
  } catch (std::exception& e) { std::cerr << e.what() << std::endl; }
}

Options ParseArgs(int argc, char* argv[]) {
  Options options;

  std::vector<std::string> args(argv + 1, argv + argc);
  for (auto it = args.begin(); it != args.end(); ++it) {
    if (*it == "-h" || *it == "--help") {
      options.print_help = true;
      break;
    } else if (*it == "--log-level") {
      ++it;
      if (it == args.end()) {
        std::cerr << "Invalid argument to '--log-level'\n";
        options.print_help = true;
        break;
      }
      // clang-format off
      else if (*it == "all") LOG_LEVEL(All);
      else if (*it == "trace") LOG_LEVEL(Trace);
      else if (*it == "debug") LOG_LEVEL(Debug);
      else if (*it == "info") LOG_LEVEL(Info);
      else if (*it == "warn") LOG_LEVEL(Warn);
      else if (*it == "error") LOG_LEVEL(Error);
      else if (*it == "none") LOG_LEVEL(None);
      else {
        std::cerr << "Invalid argument to '--log-level'\n";
        options.print_help = true;
        break;
      }
      // clang-format on
    } else if (it->front() != '-') {
      options.rom_file = std::move(*it);
    } else {
      std::cerr << "Unrecognized option '" << *it << "'\n";
      options.print_help = true;
      break;
    }
  }

  return options;
}

void PrintHelp() {
  constexpr auto help = R"EOF(usage: renes [options] [ROM]

options:
  -h, --help              Prints this help message and exits
      --log-level LEVEL   Sets the logging level to LEVEL. Can be one of:
                          'all', 'error', 'warn', 'info', 'debug', 'trace', or
                          'none'

)EOF";

  std::cout << help;
}