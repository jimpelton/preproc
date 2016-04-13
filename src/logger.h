#ifndef epic_logerror_h__
#define epic_logerror_h__

#ifndef FILE_LOG_LEVEL
#define FILE_LOG_LEVEL DEBUG
#endif  // !FILE_LOG_LEVEL

#include <ostream>
#include <iostream>
#include <chrono>

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
    }

    s_instance->m_level = level;

    return *s_instance;
  }


  /////////////////////////////////////////////////////////////////////////////
  static void shutdown()
  {
    delete s_instance;
  }

  /////////////////////////////////////////////////////////////////////////////
  template<typename T>
  logger&
  do_log(T t)
  {
    *m_out << "- " << now() << " " << m_levelString << ":\t";
    *m_out << t;
    return *this;
  }


  ~logger()
  {
    *m_out << std::endl;

    if (m_ownsStream) {
      delete m_out;
    }
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
  return logger::get(LogLevel::DEBUG);
}
inline logger& Err()
{
  return logger::get(LogLevel::ERROR);
}

inline logger& Info()
{
  return logger::get(LogLevel::INFO);
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
