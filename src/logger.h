#ifndef epic_logerror_h__
#define epic_logerror_h__

#ifndef FILE_LOG_LEVEL
#define FILE_LOG_LEVEL DEBUG
#endif  // !FILE_LOG_LEVEL

#include <ostream>
#include <iostream>
#include <chrono>
#include <thread>

namespace preproc {

enum class LogLevel
{
  INFO, DEBUG, ERROR
};

class logger
{
public:

  /////////////////////////////////////////////////////////////////////////////
  static logger &get(LogLevel level = LogLevel::INFO)
  {
    if (s_instance == nullptr) {
      s_instance = new logger();
      s_instance->do_log("Logger initialized.");
    }

    s_instance->m_level = level;

    return *s_instance;
  }


  /////////////////////////////////////////////////////////////////////////////
  static void shutdown()
  {
    get().do_log("Shutdown logger.");
    delete s_instance;
  }

  /////////////////////////////////////////////////////////////////////////////
  logger&
  start_line()
  {
    std::thread::id tid = std::this_thread::get_id();
    *m_out << "\n- " << now() << " (" << std::hex << tid << std::dec << ") " << m_levelString << ":\t";
    return *this;
  }

  /////////////////////////////////////////////////////////////////////////////
  template<typename T>
  logger&
  do_log(T t)
  {
    *m_out << t;
    return *this;
  }


  ~logger()
  {
    *m_out << "\n";
    m_out->flush();

    if (m_ownsStream) {
      delete m_out;
      m_out = nullptr;
    }

    s_instance = nullptr;

  }


private:


  /////////////////////////////////////////////////////////////////////////////
  /// \brief Create logger that logs to stdout.
  /////////////////////////////////////////////////////////////////////////////
  logger()
    : m_out{ &(std::cout) }
    , m_file{ nullptr }
    , m_ownsStream{ false }
    , m_level{ LogLevel::INFO }
    , m_levelString{ "INFO" }
  {
    switch(m_level) {
      case LogLevel::DEBUG: m_levelString = "DEBUG"; break;
      case LogLevel::ERROR: m_levelString = "ERROR"; break;
      case LogLevel::INFO:
      default: m_levelString  = "INFO"; break;
    }

//    m_out = std::cout;
  }

  std::string now()
  {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    char buf[80];
    std::strftime(buf, 80, "%F %T", std::localtime(&now));

    return std::string(buf);
  }

private:   // members
  static logger *s_instance;
  std::ostream *m_out;
  std::ostream *m_file;
  bool m_ownsStream;
  LogLevel m_level;
  const char *m_levelString;

}; // class logger


inline logger& Dbg()
{
  return logger::get(LogLevel::DEBUG).start_line();
}
inline logger& Err()
{
  return logger::get(LogLevel::ERROR).start_line();
}

inline logger& Info()
{
  return logger::get(LogLevel::INFO).start_line();
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Log it, whatever it is, it better have an operator<<()!
////////////////////////////////////////////////////////////////////////////////
template<typename T>
preproc::logger&
operator<<(preproc::logger& log, T t)
{
  return log.do_log(t);
}


} //namespace preproc
#endif  // !epic_logerror_h__
