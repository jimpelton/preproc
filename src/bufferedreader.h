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


class ReaderWorker;
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


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Start the reader thread.
  ///////////////////////////////////////////////////////////////////////////////
  bool start();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Stop reading as soon as possible.
  ///////////////////////////////////////////////////////////////////////////////
  void stop();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Fill the buffer with data.
  /// \return The number of elements (not necessarily bytes) in the buffer.
  ///////////////////////////////////////////////////////////////////////////////
//  static void fillBuffer(BufferedReader *);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief True if not at end of file.
  ///////////////////////////////////////////////////////////////////////////////
  bool hasNextFill() const { return !(m_is->eof()); }


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Reset the reader to the start of the file.
  ///////////////////////////////////////////////////////////////////////////////
  void reset();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Wait for a full buffer to be placed in the queue and return it.
  ///////////////////////////////////////////////////////////////////////////////
  const thrust::host_vector<Ty>& nextFull();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Return an empty buffer to the queue to be filled again.
  ///////////////////////////////////////////////////////////////////////////////
  void returnEmpty(const thrust::host_vector<Ty>&);


  size_t bufferSizeElements() const;


private:
  thrust::host_vector<Ty>& nextEmpty();
  void returnFull(thrust::host_vector<Ty> &);

  std::vector<thrust::host_vector<Ty>> m_allBuffers;
  std::queue<thrust::host_vector<Ty>*> m_emptyBuffers;
  std::queue<thrust::host_vector<Ty>*> m_fullBuffers;

  int m_nBufs;

  size_t m_szBytesTotal;
//  size_t m_filePos;
  std::string m_path;
  std::ifstream *m_is;

  ReaderWorker *worker;
  std::thread *m_readThread;
  std::mutex m_emptyBuffersLock;
  std::mutex m_fullBuffersLock;
  std::condition_variable_any m_emptyBuffersAvailable;
  std::condition_variable_any m_fullBuffersAvailable;

  class ReaderWorker
  {
  public:
    ReaderWorker(BufferedReader *r)
        : m_hasNext{ true }
        , m_stopRequested{ false }
        , m_reader{ r }
    { }

    void operator()()
    {
      std::ifstream *is = m_reader->m_is;

      const size_t buffer_size_elems{ m_reader->bufferSizeElements() };
      const size_t buffer_size_bytes{ m_reader->bufferSizeElements() * sizeof(Ty) };

      while(! m_stopRequested) {
        thrust::host_vector<Ty> &buf = m_reader->nextEmpty();
        Ty *data{ buf.data() };
        is->read(reinterpret_cast<char*>(data), buffer_size_bytes);
        std::streampos amount{ is->gcount() };

        // the last buffer filled may not be a full buffer, so resize!
        if (amount < buffer_size_bytes && amount > 0) {
          Info() << "Resizing last buffer to size: " << amount;
          buf.resize(amount / sizeof(Ty));
        }

        m_reader->returnFull(buf);

      }
    }

    void requestStop(){ m_stopRequested = true; }

  private:
    std::atomic<bool> m_hasNext;
    std::atomic<bool> m_stopRequested;


    BufferedReader<Ty> *m_reader;
  }; // ReaderWorker
};



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
  m_path = path;
  m_is = new std::ifstream();
  m_is->open(path, std::ios::binary);

  if ( ! m_is->is_open()) { return false; }

  for(int i=0; i < m_nBufs; ++i) {
    m_allBuffers.push_back( thrust::host_vector<Ty>{} );
    m_allBuffers.back().resize( bufferSizeElements() );
    m_emptyBuffers.push(&(m_allBuffers.back()));
  }

  Info() << "Generated " << m_allBuffers.size() << " buffers of size " <<
      bufferSizeElements();

  return true;

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
bool
BufferedReader<Ty>::start()
{
  worker = new ReaderWorker(this);
  m_readThread = new std::thread(*worker);
  return true;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::stop()
{
  worker->requestStop();
  Info() << "Waiting for reader thread to stop.";
  m_readThread->join();
}


///////////////////////////////////////////////////////////////////////////////
// static
//template<typename Ty>
//void
//BufferedReader<Ty>::fillBuffer(BufferedReader *inst)
//{
//  int next_empty{ 0 };
//  size_t buffer_size_elements{ inst->bufferSizeElements() };
//  size_t nbufs{ inst->m_nBufs };
//  std::atomic<bool> *stop_reading = &(inst->m_stopReading);
//
//  std::queue<thrust::host_vector<Ty>*> *empty_buffers{ &(inst->m_emptyBuffers) };
//  std::queue<thrust::host_vector<Ty>*> *full_buffers{ &(inst->m_fullBuffers) };
//
//}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::reset()
{
  m_is->clear();
  m_is->seekg(0, std::ios::beg);
//  m_filePos = 0;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
const thrust::host_vector<Ty>&
BufferedReader<Ty>::nextFull()
{
  m_fullBuffersLock.lock();
  while(m_fullBuffers.size() == 0) {
    m_fullBuffersAvailable.wait(m_fullBuffersLock);
  }

  auto *buf = m_fullBuffers.front();
  m_fullBuffers.pop();

  m_fullBuffersLock.unlock();

  return *buf;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::returnEmpty(const thrust::host_vector<Ty> &buf)
{
  // make sure this is one of our buffers.
  auto thing = std::find(m_allBuffers.begin(), m_allBuffers.end(), buf);
  if (thing == m_allBuffers.end()) {
    Err() << "This was not one of our buffers!";
    return;
  }

  m_emptyBuffersLock.lock();

  m_emptyBuffers.push(&*thing);
  m_emptyBuffersAvailable.notify_all();

  m_emptyBuffersLock.unlock();

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
thrust::host_vector<Ty> &
BufferedReader<Ty>::nextEmpty()
{
  m_emptyBuffersLock.lock();
  while(m_emptyBuffers.size() == 0) {
    m_emptyBuffersAvailable.wait(m_emptyBuffersLock);
  }

  thrust::host_vector<Ty> *buf{ m_emptyBuffers.front() };
  m_emptyBuffers.pop();

  m_emptyBuffersLock.unlock();

  return *buf;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::returnFull(thrust::host_vector<Ty> &buf)
{
  m_fullBuffersLock.lock();
  m_fullBuffers.push(&buf);
  m_fullBuffersAvailable.notify_all();
  m_fullBuffersLock.unlock();
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
size_t
BufferedReader<Ty>::bufferSizeElements() const
{
  return m_szBytesTotal / m_nBufs;
}



} // namespace preproc
#endif // ! bufferedreader_h__
