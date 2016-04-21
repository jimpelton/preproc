#ifndef readerworker_h__
#define readerworker_h__

#include "logger.h"
#include "bufferpool.h"
#include "bufferedreader.h"
#include "buffer.h"


#include <fstream>
#include <atomic>
#include <string>

namespace preproc
{


template<typename Reader, typename Pool, typename Ty>
class ReaderWorker
{
public:
  ReaderWorker(Reader *r, Pool *p)
//  : m_hasNext{ true }
    : m_stopRequested{ false }
    , m_reader{ r }
    , m_pool{ p }
    , m_is{ nullptr }
  { }

  int operator()()
  {
    if (! open()) {
        Err() << "RW: Could not open file " << m_path << 
            ". Exiting readerworker loop.";
        return -1;
    }

    // bytes to attempt to read from file.
    const size_t buffer_size_bytes{ m_pool->bufferSizeElements() * sizeof(Ty) };
    size_t total_read_bytes{ 0 };

    Info() << "RW: Entering loop"; 
    while(!(m_is->eof()) && !m_stopRequested) {
      Info() << "RW: Waiting for buffer in readerworker loop.";

      Buffer<Ty> *buf = m_pool->nextEmpty();
      Ty *data = buf->ptr();
      Info() << "RW: Got buffer in readerworker loop.";
      m_is->read(reinterpret_cast<char*>(data), buffer_size_bytes);
      std::streampos amount{ m_is->gcount() };
      Info() << "RW: Read " << amount << " bytes.";

      // the last buffer filled may not be a full buffer, so resize!
      if (amount < buffer_size_bytes ) {
        if (amount < 0) {
          Err() << "RW: Read < 0 bytes, breaking out of IO loop.";
          return -1;
        }
        buf->elements(amount/sizeof(Ty));
      }

      total_read_bytes += amount;

      Info() << "RW: Going to return full buffer.";
      m_pool->returnFull(buf);
      Info() << "RW: Returned full buffer.";
    } // while

    Info() << "RW: Leaving IO loop.";
    return total_read_bytes;
  }

    
  void setPath(const std::string &path)
  {
    m_path = path;
  }

//  void setFileStream(std::ifstream *s) 
//  {
//    m_is = s;
//  }


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Request that the reader stop working as soon as it can.
  ///////////////////////////////////////////////////////////////////////////////
  void requestStop() { m_stopRequested = true; }
  


private:
  bool open() 
  {
    m_is = new std::ifstream();
    m_is->open(m_path, std::ios::binary);
    if (! m_is->is_open()) { 
        return false; 
    }
    return true;
  }




  bool m_stopRequested;
  Reader *m_reader;
  Pool *m_pool;
  std::ifstream *m_is;
  std::string m_path;

}; // ReaderWorker


} //namespace preproc

#endif // ! readerworker_h__
