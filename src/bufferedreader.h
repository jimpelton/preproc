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


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Fill the buffer with data.
  /// \return The number of elements (not necessarily bytes) in the buffer.
  ///////////////////////////////////////////////////////////////////////////////
  void fillBuffer();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief True if not at end of file.
  ///////////////////////////////////////////////////////////////////////////////
  bool hasNextFill() const { return !(m_is->eof()); }


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Reset the reader to the start of the file.
  ///////////////////////////////////////////////////////////////////////////////
  void reset();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Return pointer to buffer.
  ///////////////////////////////////////////////////////////////////////////////
//  Ty *buffer_ptr() const { return m_buffer; }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get a begin/end iterator pair of the next buffer segment.
  //////////////////////////////////////////////////////////////////////////////
  auto
  next() {
    auto rval = m_buffers.front();
    m_buffers.pop();
    return rval;
  }


  size_t bufferSizeElements() const
  {
    return m_buffer.size() / m_nBufs;
  }

private:

  auto makePair(int next_empty) {
    return thrust::make_pair(
        m_buffer.begin() + (next_empty * bufferSizeElements()),
        m_buffer.begin() + ((next_empty * bufferSizeElements()) + bufferSizeElements())
    );
  }
  thrust::host_vector<Ty> m_buffer;
  std::queue<thrust::pair<
      decltype(m_buffer.begin()), decltype(m_buffer.begin())>> m_buffers;

  int m_nBufs;

  size_t m_szBytesTotal;
  size_t m_filePos;
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
  : m_szBytesTotal{ bufSize }
  , m_nBufs{ nbuf }
  , m_filePos{ 0 }
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

  if (m_is->is_open()) {
    m_buffer.resize(m_szBytesTotal / sizeof(Ty));
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::fillBuffer()
{
  Ty *h_buf{ m_buffer.data() };
  int next_empty{ 0 };

  while (! m_stopReading) {
    if (m_buffers.size() < m_nBufs) {
      size_t offset{ next_empty * bufferSizeElements() };
      h_buf = h_buf + offset;

      size_t bytes_to_read{ bufferSizeElements() * sizeof(Ty) };
      m_is->read(reinterpret_cast<char *>(h_buf), bytes_to_read);

      std::streampos amount{ m_is->gcount() };
      m_filePos += amount;
      m_buffers.push(makePair(next_empty));
      next_empty += 1;
    }
  }
}


template<typename Ty>
void
BufferedReader<Ty>::reset()
{
  m_is->clear();
  m_is->seekg(0, std::ios::beg);
  m_filePos = 0;
}

} // namespace bd



#endif // ! bufferedreader_h__
