#ifndef buffered_reader__
#define buffered_reader__

#include "readerworker.h"
#include "bufferpool.h"

#include <fstream>
#include <thread>
#include <mutex>

namespace preproc
{


template<typename Ty>
class BufferedReader 
{

public:
  BufferedReader(size_t bufSize);
  ~BufferedReader();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Open the raw file at path.
  /// \return True if opened, false otherwise.
  ///////////////////////////////////////////////////////////////////////////////
  bool open(const std::string &path);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Start readering the file.
  ///////////////////////////////////////////////////////////////////////////////
  bool start();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Stop reading as soon as possible.
  ///////////////////////////////////////////////////////////////////////////////
  void stop();

  
  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Reset file pointer to begining of file.
  ///////////////////////////////////////////////////////////////////////////////
  void reset();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief True if not at end of file.
  ///////////////////////////////////////////////////////////////////////////////
  bool hasNextFill() const { return !(m_is->eof()); }
  
private:
  std::string m_path;

  size_t m_bufSizeBytes;

  std::ifstream *m_is;
  BufferPool<Ty> *m_pool;  
  ReaderWorker<BufferedReader<Ty>, BufferPool<Ty>, Ty> *m_worker;
  std::thread *m_readThread;
};

template<typename Ty>
BufferedReader<Ty>::BufferedReader(size_t bufSize)
  : m_path{ }
  , m_bufSizeBytes{ bufSize }
  , m_is{ nullptr }
  , m_pool{ nullptr }
  , m_worker{ nullptr }
  , m_readThread{ nullptr }
{
}

template<typename Ty>
BufferedReader<Ty>::~BufferedReader()
{
  if (m_is) delete m_is;
  if (m_pool) delete m_pool;
  if (m_worker) delete m_worker;
  if (m_readThread) delete m_readThread;
} 

template<typename Ty>
bool
BufferedReader<Ty>::open(const std::string &path)
{
  m_path = path;
  m_is = new std::ifstream();
  m_is->open(path, std::ios::binary);

  if (! m_is->is_open()) { 
      return false; 
  }
  
  m_pool = new BufferPool<Ty>(m_bufSizeBytes, 4);  
  m_pool->allocate();

  return true;

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
bool
BufferedReader<Ty>::start()
{
  ReaderWorker<BufferedReader<Ty>, BufferPool<Ty>, Ty> worker(this, m_pool);
  worker.setFileStream(m_is);
  m_readThread = new std::thread(worker);
  return true;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::stop()
{
  m_worker->requestStop();
  Info() << "Waiting for reader thread to stop.";
  m_readThread->join();
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::reset()
{
  m_is->clear();
  m_is->seekg(0, std::ios::beg);
}


} // namespace preproc
#endif // ! buffered_reader__
