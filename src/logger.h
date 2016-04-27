#ifndef epic_logerror_h__
#define epic_logerror_h__

#ifndef FILE_LOG_LEVEL
#define FILE_LOG_LEVEL DEBUG
#endif  // !FILE_LOG_LEVEL

#include <ostream>
#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>

#include <sys/time.h>
#include <ctime>

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
    std::stringstream ss;
    ss << "\n- " << now() << " (" << std::hex << tid << std::dec << ") "
        << m_levelString << ":\t";

    m_writeMutex.lock();
    *m_out << ss.str();
    m_writeMutex.unlock();

    return *this;
  }

  /////////////////////////////////////////////////////////////////////////////
  template<typename T>
  logger&
  do_log(T t)
  {
    m_writeMutex.lock();
    *m_out << t;
    m_writeMutex.unlock();
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


private:  // methods


  /////////////////////////////////////////////////////////////////////////////
  /// \brief Create logger that logs to stdout.
  /////////////////////////////////////////////////////////////////////////////
  logger()
    : m_out{ &(std::cout) }
//    , m_file{ nullptr }
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
//    auto now =
//        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    timeval tv;
    time_t curtime;
    gettimeofday(&tv, nullptr);
    curtime = tv.tv_sec;
    char datetimebuf[20]; // "%F %T\0" = 20 chars
    std::strftime(datetimebuf, 20, "%F %T", std::localtime(&curtime));
    char buf2[20+8]; // "%s.%ld\0" = 28 chars
    sprintf(buf2, "%s.%d", datetimebuf, tv.tv_usec);

    return std::string(buf2);
  }

private:   // members
  static logger *s_instance;

  std::ostream *m_out;
//  std::ostream *m_file;
  bool m_ownsStream;
  LogLevel m_level;
  const char *m_levelString;

  std::mutex m_writeMutex;


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
