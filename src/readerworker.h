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

  size_t operator()()
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
    while(!(m_is->eof()) || !m_stopRequested) {
      Info() << "RW: Waiting for buffer in readerworker loop.";

      Buffer<Ty> *buf = m_pool->nextEmpty();
      buf->index(total_read_bytes / sizeof(Ty));  // element index this buffer starts at.
      Ty *data = buf->ptr();
        
      Info() << "RW: Reading into buffer.";
      m_is->read(reinterpret_cast<char*>(data), buffer_size_bytes);
      std::streampos amount{ m_is->gcount() };
      Info() << "RW: Read " << amount << " bytes.";
      
      // the last buffer filled may not be a full buffer, so resize!
      if ( amount < static_cast<long long>(buffer_size_bytes) ) {

        buf->elements(amount/sizeof(Ty));
        if (amount == 0) {
          Info() << "RW: Returning empty buffer.";
          m_pool->returnEmpty(buf);
          Info() << "RW: Returned empty buffer.";
          break;
        }
      } // if (amount...)
        
      Info() << "RW: Going to return full buffer.";
      m_pool->returnFull(buf);
      Info() << "RW: Returned full buffer.";

      total_read_bytes += amount;
    } // while

    Info() << "RW: Leaving IO loop after reading " << total_read_bytes << " bytes";
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
    return m_is->is_open();
  }




  bool m_stopRequested;
  Reader *m_reader;
  Pool *m_pool;
  std::ifstream *m_is;
  std::string m_path;

}; // ReaderWorker


} //namespace preproc

#endif // ! readerworker_h__
