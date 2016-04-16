//
// Created by jim on 4/10/16.
//

#ifndef bufferedreader_h__
#define bufferedreader_h__


#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cassert>

#include <thrust/host_vector.h>
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

  ////////////////////////////////////////////////////////////////////////////////
  BufferedReader(size_t bufSize, int numBuffers=2);


  ~BufferedReader();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Open the raw file at path.
  ///////////////////////////////////////////////////////////////////////////////
  bool open(const std::string &path);

  bool start();
  bool stop();

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


  using ThrustNormalIter = thrust::detail::normal_iterator<Ty>;
  using BuffersPair = thrust::pair<ThrustNormalIter, ThrustNormalIter>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get a begin/end iterator pair of the next buffer segment.
  //////////////////////////////////////////////////////////////////////////////
  BuffersPair
  next()  {
    m_mutex.lock();

    auto rval = m_buffers.front();
    m_buffers.pop();

    m_mutex.unlock();

    m_cv.notify_all();
    return rval;
  }


  size_t bufferSizeElements() const
  {
    return m_buffer.size() / m_nBufs;
  }


private:   // Methods

  BuffersPair
  makePair(int next, std::streampos gofor) {
    return thrust::make_pair(
        m_buffer.begin() + (next * bufferSizeElements()),
        m_buffer.begin() + ((next * bufferSizeElements()) + gofor)
    );
  }


private:   // Data members

  thrust::host_vector<Ty> m_buffer;

  using BuffersQueue = std::queue<thrust::pair<decltype(m_buffer.begin()), decltype(m_buffer.begin())>>;
  BuffersQueue m_buffers;

  int m_nBufs;

  size_t m_szBytesTotal;
//  size_t m_filePos;
  std::string m_path;
  std::ifstream *m_is;
  std::atomic<bool> m_stopReading;

  std::thread m_readThread;
  std::mutex m_mutex;
  std::mutex m_buffersMutex;
  std::condition_variable_any m_cv;

};  // class BufferedReader


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BufferedReader<Ty>::BufferedReader
(
  size_t bufSize,
  int nbuf
)
//  : m_buffer{ nullptr }
  : m_buffer{ }
  , m_buffers{ }
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

  if (m_is->is_open()) {
    m_buffer.resize(m_szBytesTotal / sizeof(Ty));
    return true;
  }

  return false;
}


template<typename Ty>
bool
BufferedReader<Ty>::start()
{
  m_stopReading = false;
  m_readThread(this->fillBuffer, this);
  return true;
}

template<typename Ty>
bool
BufferedReader<Ty>::stop()
{
  m_stopReading = true;
}

///////////////////////////////////////////////////////////////////////////////
// static
template<typename Ty>
void
BufferedReader<Ty>::fillBuffer(BufferedReader *inst)
{
  Ty *h_buf{ inst->m_buffer.data() };
  int next_empty{ 0 };
  size_t buffer_size_elements{ inst->bufferSizeElements() };

  while (! inst->m_stopReading) {
    while (inst->m_buffers.size() >= inst->m_nBufs) {
      inst->m_cv.wait(inst->m_buffersMutex);
    }

    h_buf = h_buf + (next_empty * buffer_size_elements);

    inst->m_is->read(reinterpret_cast<char *>(h_buf), buffer_size_elements * sizeof(Ty));

    std::streampos amount{ inst->m_is->gcount() };
//    inst->m_filePos += amount;

    inst->m_mutex.lock();
    inst->m_buffers.push(inst->makePair(next_empty, amount/sizeof(Ty)));
    inst->m_mutex.unlock();

    next_empty = (next_empty + 1) % inst->m_nBufs;

  }
}


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
