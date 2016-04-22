#ifndef minmax_h__
#define minmax_h__

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <limits>

template<typename Ty>
class MinMax {
private: 
  const Ty *const array;

public:
  Ty min_value;
  Ty max_value;

  void operator()( const tbb::blocked_range<size_t>& r )
  {
    const Ty *a = array;
    Ty val;
    for(size_t i{ r.begin() }; i != r.end(); ++i) {
      val = a[i];
      if (val < min_value) { min_value = val; }
      if (val > max_value) { max_value = val; }
    }
  }

  MinMax( MinMax &x, tbb::split )
      : array{ x.array }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::min() }
  { }

  void join(const MinMax &y)
  {
    if (y.min_value < min_value) {
      min_value = y.min_value;
    }
    if (y.max_value > max_value) {
      max_value = y.max_value;
    }
  }

  MinMax(const Ty *a)
      : array{ a }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::min() }
  { }

};


#endif // ! minmax_h__
