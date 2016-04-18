#ifndef preproc_buffer_h__
#define preproc_buffer_h__

#include <cstddef>

template<typename Ty>
class Buffer
{
public:
  Buffer(Ty* start, size_t nElems) 
    : m_start{ start }
    , m_elementLength{ nElems }
  { }

  bool operator==(const Buffer<Ty>& rhs) 
  {
    return this->m_start == rhs.m_start;
  }

  /// \brief Get a pointer to the start of this buffer's memory.
  Ty* ptr() { return m_start; }
  const Ty* ptr() const { return m_start; }

  /// \brief Get the size in elements of this buffer.
  void elements(size_t l) { m_elementLength = l; }
  size_t elements() const { return m_elementLength; }


private:
    Ty* m_start;
    size_t m_elementLength;


}; // Buffer

#endif // ! preproc_buffer_h__
