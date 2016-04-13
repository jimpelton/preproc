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
  BufferedReader(size_t bufSize);


  ~BufferedReader();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Open the raw file at path.
  ///////////////////////////////////////////////////////////////////////////////
  bool open(const std::string &path);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Fill the buffer with data.
  /// \return The number of elements (not necessarily bytes) in the buffer.
  ///////////////////////////////////////////////////////////////////////////////
  size_t fillBuffer();


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
  Ty *buffer_ptr() const { return m_buffer; }

  size_t bufferSizeElements() { return m_bufSize/sizeof(Ty); }

private:
  Ty *m_buffer;
  size_t m_bufSize;
  std::streamsize fileSize;
  std::size_t m_filePos;
  std::string m_path;
  std::ifstream *m_is;

};  // class BufferedReader


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BufferedReader<Ty>::BufferedReader
(
  size_t bufSize
)
  : m_buffer{ nullptr }
  , m_bufSize{ bufSize }
  , m_filePos{ 0 }
  , m_path{ }
  , m_is{ nullptr }
{ }


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BufferedReader<Ty>::~BufferedReader()
{
  if (m_is) delete m_is;
  if (m_buffer) delete[] m_buffer;
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
    m_buffer = new Ty[m_bufSize];
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
size_t
BufferedReader<Ty>::fillBuffer()
{
  m_is->read(reinterpret_cast<char *>(m_buffer), m_bufSize);
  std::streampos amount{ m_is->gcount() };
  m_filePos += amount;
  return static_cast<size_t>(amount / sizeof(Ty));
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
