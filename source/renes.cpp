#include <future>
#include <iostream>
#include <string_view>
#include <thread>
#include <vector>

#include "gui/gui.hpp"
#include "nes/nes.hpp"

wxIMPLEMENT_APP_NO_MAIN(gui::Application);

struct Options {
  bool print_help = false;
  nes::LogLevel log_level = nes::LogLevel::Info;
  std::string log_file = "";
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

  if (!options.log_file.empty()) { LOG_FILE(options.log_file); }

  auto console = nes::Console{};

  auto gui_thread = std::thread{RunGui, argc, argv, &console};
  auto nes_thread = std::thread{RunNes, options, &console};

  gui_thread.join();
  nes_thread.join();

  return 0;
}

// ----------------------------------------------
// Definitions
// ----------------------------------------------

void RunGui(int argc, char* argv[], nes::Console* console) {
  auto app = new gui::Application{console};
  wxApp::SetInstance(app);
  wxEntry(argc, argv);
}

void RunNes(Options const& options, nes::Console* console) {
  LOG_INFO("Starting ReNES");

  console->Reset();
  if (!options.rom_file.empty()) {
    console->Load(options.rom_file);
  }  
  
  console->Run();
}

Options ParseArgs(int argc, char* argv[]) {
  using namespace std::literals;

  auto options = Options{};
  std::vector<std::string_view> args(argv + 1, argv + argc);

  auto IsFlag = [](auto it) { return it.front() == '-'; };

  auto InvalidArgument = [&](std::string_view flag, std::string_view arg = ""sv) mutable {
    options.print_help = true;
    std::cerr << "Invalid argument to '" << flag;
    if (arg.empty()) {
      std::cerr << "'\n";
      return;
    } else {
      std::cerr << "' : '" << arg << "'\n";
    }
  };

  auto GetArgument = [&](auto flag, auto it) {
    if (it == args.end() || IsFlag(*it)) {
      options.print_help = true;
      InvalidArgument(flag);
      return ""sv;
    }
    return *it;
  };

  auto UnknownFlag = [&](std::string_view flag) mutable {
    options.print_help = true;
    std::cerr << "Unknown flag '" << flag << "'\n";
  };

  for (auto it = args.begin(); it != args.end(); ++it) {
    if (auto flag = *it; IsFlag(flag)) {
      if (flag == "-h" || flag == "--help") {
        options.print_help = true;
        return options;
      }

      auto arg = GetArgument(flag, ++it);
      if (arg.empty()) return options;

      if (flag == "--log-level") {
        // clang-format off
        if (arg == "all") LOG_LEVEL(All);
        else if (arg == "trace") LOG_LEVEL(Trace);
        else if (arg == "debug") LOG_LEVEL(Debug);
        else if (arg == "info") LOG_LEVEL(Info);
        else if (arg == "warn") LOG_LEVEL(Warn);
        else if (arg == "error") LOG_LEVEL(Error);
        else if (arg == "none") LOG_LEVEL(None);
        else InvalidArgument(flag, arg);
        // clang-format on
      } else if (flag == "--log-file") {
        options.log_file = arg;
      } else {
        UnknownFlag(flag);
        return options;
      }
    } else /* !IsFlag(it) */ {
      if (options.rom_file.empty()) {
        options.rom_file = *it;
      } else {
        std::cerr << *it << " is not a valid position argument\n";
        options.print_help = true;
        return options;
      }
    }
  }
  return options;
}

void PrintHelp() {
  constexpr auto help = R"EOF(
usage: renes [options] [rom]

options:
  -h, --help              Prints this help message and exits.
      --log-file  FILE    Logs output to FILE instead of to the console.
      --log-level LEVEL   Sets the logging level to LEVEL. Can be one of:
                          'all', 'error', 'warn', 'info', 'debug', 'trace',
                          or 'none'

)EOF";

  std::cout << help;
}