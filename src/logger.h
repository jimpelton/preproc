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


class logger
{
public:

  /////////////////////////////////////////////////////////////////////////////
  /// \brief class sentry
  /////////////////////////////////////////////////////////////////////////////
  class sentry
  {
    logger &log;

  public:
    sentry(logger &l)
      : log(l)
    {
      log.lock();
      log.start_line();
    }

    ~sentry()
    {
      log.end_line();
      log.unlock();
    }


    template<typename T>
    sentry& operator<<(T t)
    {
      log.do_log(t);
      return *this;
    }


    sentry& operator<<(std::ostream & (*man)(std::ostream &))
    {
      // Handles manipulators like endl.
      log.do_log(man);
      return *this;
    }


    sentry& operator<<(std::ios_base & (*man)(std::ios_base &))
    {
      log.do_log(man);
      // Handles manipulators like hex.
      return *this;
    }

  }; // sentry


  /////////////////////////////////////////////////////////////////////////////
  /// \brief enum class LogLevel
  /////////////////////////////////////////////////////////////////////////////
  enum class LogLevel
  {
    INFO, DEBUG, ERROR
  };


  /////////////////////////////////////////////////////////////////////////////
  /// \brief Create logger that logs to stdout.
  /////////////////////////////////////////////////////////////////////////////
  logger()
    : m_out{ std::cout }
    , m_level{ LogLevel::INFO }
    , m_levelString{ "INFO" }
    //, m_guard{ m_mutex, std::defer_lock }
  {
    switch(m_level) {
    case LogLevel::DEBUG: m_levelString = "DEBUG"; break;
    case LogLevel::ERROR: m_levelString = "ERROR"; break;
    case LogLevel::INFO:
    default: m_levelString  = "INFO"; break;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /// \brief And new line and flush output stream.
  /////////////////////////////////////////////////////////////////////////////
  ~logger()
  {
    m_out << "\n";
    m_out.flush();

    s_instance = nullptr;

  }

  /////////////////////////////////////////////////////////////////////////////
  static logger& get(LogLevel level = LogLevel::INFO)
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
    get().do_log("\nShutdown logger.");
    delete s_instance;
  }

  /////////////////////////////////////////////////////////////////////////////
  void start_line()
  {
    std::thread::id tid = std::this_thread::get_id();
    //std::stringstream ss;
    m_out << "- " << now() << " (" << std::hex << tid << std::dec << ") "
        << m_levelString << ":\t";

    //m_out << ss.str();
  }

  /////////////////////////////////////////////////////////////////////////////
  void end_line()
  {
    m_out << std::endl;
  }


  /////////////////////////////////////////////////////////////////////////////
  template<typename T>
  void do_log(T t)
  {
    m_out << t;
  }


  /////////////////////////////////////////////////////////////////////////////
  template<typename T>
  sentry operator<<(T t)
  {
    return sentry(*this) << t;
  }

  void lock()
  {
    m_mutex.lock();
  }

  void unlock()
  {
    m_mutex.unlock();
  }



private:
  /////////////////////////////////////////////////////////////////////////////
  std::string
  now()
  {
    timeval tv;
    time_t curtime;
    gettimeofday(&tv, nullptr);
    curtime = tv.tv_sec;
    char datetimebuf[20]; // "%F %T\0" = 20 chars
    std::strftime(datetimebuf, 20, "%F %T", std::localtime(&curtime));
    char buf2[20+8]; // "%s.%ld\0" = 28 chars

#ifdef __APPLE__
    sprintf(buf2, "%s.%d", datetimebuf, tv.tv_usec);
#else
    sprintf(buf2, "%s.%ld", datetimebuf, tv.tv_usec);
#endif

    return std::string(buf2);
  }

private:   // members

  static logger *s_instance;

  std::ostream &m_out;
  LogLevel m_level;
  const char *m_levelString;

  std::mutex m_mutex;
  //std::unique_lock<std::mutex> m_guard;


}; // class logger


inline logger& Dbg()
{
  return logger::get(logger::LogLevel::DEBUG); //.start_line();
}
inline logger& Err()
{
  return logger::get(logger::LogLevel::ERROR); //.start_line();
}

inline logger& Info()
{
  return logger::get(logger::LogLevel::INFO); //.start_line();
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Log it, whatever it is, it better have an operator<<()!
////////////////////////////////////////////////////////////////////////////////
//template<typename T>
//preproc::logger&
//operator<<(preproc::logger& log, T t)
//{
//  return log.do_log(t);
//}


} //namespace preproc
#endif  // !epic_logerror_h__
