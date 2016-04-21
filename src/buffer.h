#ifndef preproc_buffer_h__
#define preproc_buffer_h__

#include <cstddef>

template<typename Ty>
class Buffer
{
public:
  Buffer(Ty* data, size_t nElems, size_t elementIndex = 0) 
    : m_data{ data }
    , m_elementIndexOffset{ elementIndex }
    , m_elementLength{ nElems }
  { }

  bool operator==(const Buffer<Ty>& rhs) 
  {
    return this->m_data == rhs.m_data;
  }

  /// \brief Get a pointer to the start of this buffer's memory.
  Ty* ptr() { return m_data; }
  const Ty* ptr() const { return m_data; }

  /// \brief Get the size in elements of this buffer.
  void elements(size_t l) { m_elementLength = l; }
  size_t elements() const { return m_elementLength; }

  size_t index() const { return m_elementIndexOffset; }
  void index(size_t ele) { m_elementIndexOffset = ele; }

private:
    Ty* m_data;  ///< Ptr to memory of this buffer.
    size_t m_elementIndexOffset;  ///< Starting index in data stream of this buffer.
    size_t m_elementLength; ///< Length of buffer in elements.

}; // Buffer

#endif // ! preproc_buffer_h__
