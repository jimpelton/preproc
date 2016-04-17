//
// Created by jim on 4/10/16.
//

#ifndef bufferedreader_h__
#define bufferedreader_h__

#include "logger.h"

#include <thrust/host_vector.h>
#include <thrust/pair.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>

namespace preproc
{



///////////////////////////////////////////////////////////////////////////////
/// \brief Read blocks of data
///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
class BufferedReader
{
public:

//  using ThrustNormalIter = thrust::detail::normal_iterator<Ty>;
//  using BuffersPair = thrust::pair<ThrustNormalIter, ThrustNormalIter>;
//  using BuffersQueue = std::queue<BuffersPair>;

  ////////////////////////////////////////////////////////////////////////////////
  BufferedReader(size_t bufSize, int numBuffers=2);


  ~BufferedReader();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Open the raw file at path.
  ///////////////////////////////////////////////////////////////////////////////
  bool open(const std::string &path);

  bool start();
  void stop();

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Fill the buffer with data.
  /// \return The number of elements (not necessarily bytes) in the buffer.
  ///////////////////////////////////////////////////////////////////////////////
  static void fillBuffer(BufferedReader *);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief True if not at end of file.
  ///////////////////////////////////////////////////////////////////////////////
  bool hasNextFill() const { return !(m_is->eof()); }


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Reset the reader to the start of the file.
  ///////////////////////////////////////////////////////////////////////////////
  void reset();



  const thrust::host_vector<Ty>& next();
  void giveback(const thrust::host_vector<Ty>&);


  size_t bufferSizeElements() const;


private:
  std::vector<thrust::host_vector<Ty>> m_allBuffers;
  std::queue<thrust::host_vector<Ty>*> m_emptyBuffers;
  std::queue<thrust::host_vector<Ty>*> m_fullBuffers;

  int m_nBufs;

  size_t m_szBytesTotal;
//  size_t m_filePos;
  std::string m_path;
  std::ifstream *m_is;
  std::atomic<bool> m_stopReading;

  class ReaderWorker;
  ReaderWorker *worker;
  std::thread m_readThread;
  std::mutex m_emptyBuffersMutex;
  std::mutex m_fullBuffersMutex;
  std::mutex m_cvMutex;
  std::condition_variable_any m_cv;

};



class ReaderWorker
{
public:

  template<typename Ty>
  void
  operator()(BufferedReader<Ty> *br)
  {


  }
};


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
const thrust::host_vector<Ty>&
BufferedReader<Ty>::next()
{
  m_fullBuffersMutex.lock();

  auto rval = m_fullBuffers.front();
  m_fullBuffers.pop();

  m_fullBuffersMutex.unlock();

  return *rval;
}

template<typename Ty>
void
BufferedReader<Ty>::giveback(const thrust::host_vector<Ty> &buf)
{
  // make sure this is one of our buffers.
  auto thing = std::find(m_allBuffers.begin(), m_allBuffers.end(), buf);
  if (thing == m_allBuffers.end()) {
    Err() << "This was not one of our buffers!";
    return;
  }

  m_emptyBuffersMutex.lock();
  m_emptyBuffers.push(&*thing);
  m_emptyBuffersMutex.unlock();

  m_cv.notify_all();
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
size_t
BufferedReader<Ty>::bufferSizeElements() const
{
  return m_szBytesTotal / m_nBufs;
}




///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BufferedReader<Ty>::BufferedReader(size_t bufSize, int nbuf)
  : m_emptyBuffers{ }
  , m_nBufs{ nbuf }
  , m_szBytesTotal{ bufSize }
//  , m_filePos{ 0 }
  , m_path{ }
  , m_is{ nullptr }
{ }


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BufferedReader<Ty>::~BufferedReader()
{
  if (m_is) delete m_is;
//  if (m_buffer) delete[] m_buffer;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
bool
BufferedReader<Ty>::open(const std::string &path)
{
  if (m_stopReading == true) {
    return false;
  }
  m_path = path;
  m_is = new std::ifstream();
  m_is->open(path, std::ios::binary);

  if ( ! m_is->is_open()) { return false; }

  for(int i=0; i < m_nBufs; ++i) {
    m_allBuffers.push_back( thrust::host_vector<Ty>{} );
    m_allBuffers.back().resize( bufferSizeElements() );
    m_emptyBuffers.push(&(m_allBuffers.back()));
  }

  Info() << "Generated " << m_allBuffers.size() << " buffers of size " << bufferSizeElements();

  return true;

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
bool
BufferedReader<Ty>::start()
{
  m_stopReading = false;
  m_readThread = std::thread(BufferedReader::fillBuffer, this);
  return true;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::stop()
{
  m_stopReading = true;
  m_readThread.join();
}


///////////////////////////////////////////////////////////////////////////////
// static
template<typename Ty>
void
BufferedReader<Ty>::fillBuffer(BufferedReader *inst)
{
  int next_empty{ 0 };
  size_t buffer_size_elements{ inst->bufferSizeElements() };
  size_t nbufs{ inst->m_nBufs };
  std::atomic<bool> *stop_reading = &(inst->m_stopReading);

  std::queue<thrust::host_vector<Ty>*> *empty_buffers{ &(inst->m_emptyBuffers) };
  std::queue<thrust::host_vector<Ty>*> *full_buffers{ &(inst->m_fullBuffers) };

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::reset()
{
  m_is->clear();
  m_is->seekg(0, std::ios::beg);
//  m_filePos = 0;
}

} // namespace bd



#endif // ! bufferedreader_h__
