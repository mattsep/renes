#pragma once

#include <fstream>
#include <iostream>
#include <mutex>

#define LOG_LAZILY(expr) [&]() -> decltype(auto) { return expr; }
#define LOG_FILE(file)   ::nes::Log::SetFile(file)
#define LOG_LEVEL(level) ::nes::Log::SetLevel(::nes::LogLevel::level)
#define LOG_TRACE(expr)  ::nes::Log::Trace(LOG_LAZILY(expr))
#define LOG_DEBUG(expr)  ::nes::Log::Debug(LOG_LAZILY(expr))
#define LOG_INFO(expr)   ::nes::Log::Info(LOG_LAZILY(expr))
#define LOG_WARN(expr)   ::nes::Log::Warn(LOG_LAZILY(expr))
#define LOG_ERROR(expr)  ::nes::Log::Error(LOG_LAZILY(expr))

namespace nes {

enum class LogLevel : int { None = 0, Error, Warn, Info, Debug, Trace, All, Default = Info };

#if !defined(RENES_ENABLE_LOGGING)

template <class Stream>
class Log {
public:
  Log(Stream &&) Log(Stream &&, LogLevel) {}

  static void SetFile(std::string const &) {}

  static void SetLevel(LogLevel) {}
  static auto GetLevel() -> LogLevel { return LogLevel::None; }

  static void Trace(...) {}
  static void Debug(...) {}
  static void Info(...) {}
  static void Warn(...) {}
  static void Error(...) {}
};

#else

class Log {
public:
  static void SetFile(std::string const& file) {
    if (file == "stdout") {
      m_buf = std::cout.rdbuf();
    } else if (file == "stderr") {
      m_buf = std::cerr.rdbuf();
    } else if (file == "stdlog") {
      m_buf = std::clog.rdbuf();
    } else {
      m_file = std::ofstream{file};
      m_buf = m_file.rdbuf();
    }
  }

  static void SetLevel(LogLevel level) {
    m_level = static_cast<int>(level);
    if (m_level > 6) m_level = 6;  // LogLevel::All
    if (m_level < 0) m_level = 0;  // LogLevel::None
  }

  static auto GetLevel() -> LogLevel { return static_cast<LogLevel>(m_level); }

  template <class... Messages>
  static void Trace(Messages&&... messages) {
    if (m_level >= 5) WriteToStream("[TRACE] : ", std::forward<Messages>(messages)...);
  }

  template <class... Messages>
  static void Debug(Messages&&... messages) {
    if (m_level >= 4) WriteToStream("[DEBUG] : ", std::forward<Messages>(messages)...);
  }

  template <class... Messages>
  static void Info(Messages&&... messages) {
    if (m_level >= 3) WriteToStream("[INFO ] : ", std::forward<Messages>(messages)...);
  }

  template <class... Messages>
  static void Warn(Messages&&... messages) {
    if (m_level >= 2) WriteToStream("[WARN ] : ", std::forward<Messages>(messages)...);
  }

  template <class... Messages>
  static void Error(Messages&&... messages) {
    if (m_level >= 1) WriteToStream("[ERROR] : ", std::forward<Messages>(messages)...);
  }

private:
  inline static int m_level = static_cast<int>(LogLevel::Default);
  inline static std::mutex m_mutex = {};
  inline static std::streambuf* m_buf = std::clog.rdbuf();
  inline static std::ofstream m_file = {};

  template <class... Messages>
  static void WriteToStream(Messages&&... messages) {
    std::lock_guard lock{m_mutex};
    std::ostream out(m_buf);
    (out << ... << CallIfNeeded(std::forward<Messages>(messages))) << '\n';
  }

  template <class Expr>
  static auto CallIfNeeded(Expr&& expr) -> decltype(auto) {
    if constexpr (std::is_invocable_v<Expr>) {
      return expr();
    } else {
      return expr;
    }
  }
};

#endif

}  // namespace nes