#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

using namespace tbb;

template<typename Ty>
struct Average {
  const Ty* input;
  Ty* output;
  void operator()( const blocked_range<int>& range ) const {
    for( int i=range.begin(); i!=range.end(); ++i )
      output[i] = (input[i-1]+input[i]+input[i+1])*(1/3.f);
  }
};

// Note: Reads input[0..n] and writes output[1..n-1]. 
template<typename Ty>
void ParallelAverage( float* output, const Ty* input, size_t n ) {
  Average<Ty> avg;
  avg.input = input;
  avg.output = output;
  parallel_for( blocked_range<int>( 1, n ), avg );
}
