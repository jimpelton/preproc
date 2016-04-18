#ifndef readerworker_h__
#define readerworker_h__

#include "logger.h"
#include "bufferpool.h"
#include "bufferedreader.h"
#include "buffer.h"


#include <fstream>
#include <atomic>

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

  void operator()()
  {
    std::ifstream *is = m_is;
    // bytes to attempt to read from file.
    const size_t buffer_size_bytes{ m_pool->bufferSizeElements() * sizeof(Ty) };

    while(! m_stopRequested) {
      Buffer<Ty> &buf = m_pool->nextEmpty();
      is->read(reinterpret_cast<char*>(buf.ptr()), buffer_size_bytes);
      std::streampos amount{ is->gcount() };

      // the last buffer filled may not be a full buffer, so resize!
      if (amount < buffer_size_bytes && amount > 0) {
//      Info() << "Resizing last buffer to size: " << amount;
        buf.elements(amount/sizeof(Ty));
      }

      m_pool->returnFull(buf);

    }
  }

  void setFileStream(std::ifstream *s) 
  {
    m_is = s;
  }


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Request that the reader stop working as soon as it can.
  ///////////////////////////////////////////////////////////////////////////////
  void requestStop() { m_stopRequested = true; }
  


private:
  bool m_stopRequested;


  Reader *m_reader;
  Pool *m_pool;
  std::ifstream *m_is;

}; // ReaderWorker


} //namespace preproc

#endif // ! readerworker_h__
