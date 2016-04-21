#ifndef minmax_h__
#define minmax_h__

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <limits>

template<typename Ty>
class MinMax {
private: 
  const float *const array;

public:
  float min_value;
  float max_value;

  void operator()( const tbb::blocked_range<Ty*>& r )
  {

  }

};


#endif // ! minmax_h__
