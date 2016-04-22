#ifndef minmax_h__
#define minmax_h__

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <limits>
#include "buffer.h"

namespace preproc
{

template<typename Ty>
class ParallelMinMax
{
private:
  //const Ty *const array;
  const Buffer<Ty> *m_buf;

public:
  Ty min_value;
  Ty max_value;

  void operator()(const tbb::blocked_range<size_t> &r)
  {
    const Ty *a = m_buf->ptr();
    size_t voxel_start = m_buf->index();

    Ty val;
    for (size_t i{ r.begin() }; i != r.end(); ++i) {
      val = a[i];
      if (val < min_value) { min_value = val; }
      if (val > max_value) { max_value = val; }
    }
  }

  ParallelMinMax(ParallelMinMax &x, tbb::split)
      : m_buf{ x.m_buf }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::min() }
  { }

  void join(const ParallelMinMax &y)
  {
    if (y.min_value < min_value) {
      min_value = y.min_value;
    }
    if (y.max_value > max_value) {
      max_value = y.max_value;
    }
  }

  ParallelMinMax(const preproc::Buffer<Ty> *b)
      : m_buf{ b }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::min() }
  { }

};

}
#endif // ! minmax_h__
