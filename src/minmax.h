#ifndef minmax_h__
#define minmax_h__

#include "buffer.h"
#include "volume.h"
#include "indexfile.h"

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include <limits>
#include <vector>

namespace preproc
{

template<typename Ty>
class ParallelMinMax
{
private:
  const Ty * const data;

public:
  Ty min_value;
  Ty max_value;

  void operator()(const tbb::blocked_range<size_t> &r)
  {
    const Ty * const a{ data };
    
    for (size_t i{ r.begin() }; i != r.end(); ++i) {

      Ty val{ a[i] };

      if (val < min_value) { min_value = val; }
      if (val > max_value) { max_value = val; }
    }
  }

  ParallelMinMax(ParallelMinMax &x, tbb::split)
      : data{ x.data }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::min() }
  { }

  void join(const ParallelMinMax &y)
  {
      // Reduce to a global minimum and maximum for the volume.
    if (y.min_value < min_value) {
      min_value = y.min_value;
    }
    if (y.max_value > max_value) {
      max_value = y.max_value;
    }
  }

  ParallelMinMax(const preproc::Buffer<Ty> *b)
      : data{ b->ptr() }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::min() }
  { }

}; // class ParallelMinMax

} // namespace preproc

#endif // ! minmax_h__
