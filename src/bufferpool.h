//
// Created by jim on 4/10/16.
//

#ifndef bufferpool_h__
#define bufferpool_h__

#include "logger.h"
#include "buffer.h"

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace preproc
{


///////////////////////////////////////////////////////////////////////////////
/// \brief Read blocks of data
///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
class BufferPool
{
public:

//  using ThrustNormalIter = thrust::detail::normal_iterator<Ty>;
//  using BuffersPair = thrust::pair<ThrustNormalIter, ThrustNormalIter>;
//  using BuffersQueue = std::queue<BuffersPair>;

  ////////////////////////////////////////////////////////////////////////////////
  BufferPool(size_t bufSize, int numBuffers);


  ~BufferPool();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Allocate the memory for this BufferPool.
  ///////////////////////////////////////////////////////////////////////////////
  void allocate();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Fill the buffer with data.
  /// \return The number of elements (not necessarily bytes) in the buffer.
  ///////////////////////////////////////////////////////////////////////////////
//  static void fillBuffer(BufferPool *);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Reset the reader to the start of the file.
  ///////////////////////////////////////////////////////////////////////////////
  void reset();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Wait for a full buffer to be placed in the queue and return it.
  ///////////////////////////////////////////////////////////////////////////////
  const Buffer<Ty>& nextFull();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Return an empty buffer to the queue to be filled again.
  ///////////////////////////////////////////////////////////////////////////////
  void returnEmpty(Buffer<Ty>&);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Get an empty buffer from the queue.
  ///////////////////////////////////////////////////////////////////////////////
  Buffer<Ty>& nextEmpty();
  

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Return a full buffer to the queue.
  ///////////////////////////////////////////////////////////////////////////////
  void returnFull(Buffer<Ty>&);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Return the size in elements of the Buffers.
  /// \note This may not be the actual length in elements of each Buffer, 
  ///       since the buffer may not have been entirely filled.
  ///////////////////////////////////////////////////////////////////////////////
  size_t bufferSizeElements() const;


private:

  Ty *m_mem;
  std::vector<Buffer<Ty>> m_allBuffers;
  std::queue<Buffer<Ty>*> m_emptyBuffers;
  std::queue<Buffer<Ty>*> m_fullBuffers;

  int m_nBufs;

  size_t m_szBytesTotal;
//  size_t m_filePos;

  std::mutex m_emptyBuffersLock;
  std::mutex m_fullBuffersLock;
  std::condition_variable_any m_emptyBuffersAvailable;
  std::condition_variable_any m_fullBuffersAvailable;

};



///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BufferPool<Ty>::BufferPool(size_t bufSize, int nbuf)
  : m_emptyBuffers{ }
  , m_nBufs{ nbuf }
  , m_szBytesTotal{ bufSize }
{ }


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BufferPool<Ty>::~BufferPool()
{
  if (m_mem) delete[] m_mem;
}

template<typename Ty>
void
BufferPool<Ty>::allocate()
{
  size_t buffer_size_elems{ bufferSizeElements() };
  m_mem = new Ty[ buffer_size_elems * sizeof(Ty) ];
  
  for(int i=0; i < m_nBufs; ++i) {
    size_t offset{ i * buffer_size_elems };
    Ty *start{ m_mem + offset };

    m_allBuffers.push_back( Buffer<Ty>{ start, buffer_size_elems } );
    m_emptyBuffers.push( &(m_allBuffers.back()) );
  }

  Info() << "Generated " << m_allBuffers.size() << 
    " buffers of size " << buffer_size_elems;

}
///////////////////////////////////////////////////////////////////////////////
// static
//template<typename Ty>
//void
//BufferPool<Ty>::fillBuffer(BufferPool *inst)
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
const Buffer<Ty>&
BufferPool<Ty>::nextFull()
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
BufferPool<Ty>::returnEmpty(Buffer<Ty> &buf)
{
  // make sure this is one of our buffers.
//  auto thing = std::find(m_allBuffers.begin(), m_allBuffers.end(), buf);
//  if (thing == m_allBuffers.end()) {
//    Err() << "This was not one of our buffers!";
//    return;
//  }

  m_emptyBuffersLock.lock();

  m_emptyBuffers.push(&buf);
  m_emptyBuffersAvailable.notify_all();

  m_emptyBuffersLock.unlock();

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
Buffer<Ty>&
BufferPool<Ty>::nextEmpty()
{
  m_emptyBuffersLock.lock();
  while(m_emptyBuffers.size() == 0) {
    m_emptyBuffersAvailable.wait(m_emptyBuffersLock);
  }

  Buffer<Ty> *buf{ m_emptyBuffers.front() };
  m_emptyBuffers.pop();

  m_emptyBuffersLock.unlock();

  return *buf;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferPool<Ty>::returnFull(Buffer<Ty> &buf)
{
  m_fullBuffersLock.lock();
  m_fullBuffers.push(&buf);
  m_fullBuffersAvailable.notify_all();
  m_fullBuffersLock.unlock();
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
size_t
BufferPool<Ty>::bufferSizeElements() const
{
  return m_szBytesTotal / m_nBufs;
}



} // namespace preproc

#endif // ! bufferpool_h__ 
