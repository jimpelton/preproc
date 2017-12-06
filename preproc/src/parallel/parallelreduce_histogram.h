//
// Created by Jim Pelton on 8/24/16.
//

#ifndef preproc_parallelhistogram_h__
#define preproc_parallelhistogram_h__

#include <bd/io/buffer.h>
#include <tbb/tbb.h>
#include <limits>
namespace preproc
{
namespace
{

const long long NUM_BUCKETS{ 1536 };
const long long MAX_IDX{ NUM_BUCKETS - 1 };

}

template<class Ty>
class ParallelReduceHistogram
{
public:
  ParallelReduceHistogram(bd::Buffer<Ty> *b, Ty rawmin, Ty rawmax)
      : m_data{ b->getPtr() }
        , m_rawmin{ rawmin }
        , m_rawmax{ rawmax }
        , m_totalCount{ 0 }
  {
    m_buckets = new unsigned int[ NUM_BUCKETS ];
    m_histMin = new Ty[ NUM_BUCKETS ];
    m_histMax = new Ty[ NUM_BUCKETS ];

    for(int i{ 0 }; i < NUM_BUCKETS; ++i){
      m_buckets[i] = 0;
      m_histMin[i] = std::numeric_limits<Ty>::max();
      m_histMax[i] = std::numeric_limits<Ty>::lowest();
    }
  }

  ParallelReduceHistogram(ParallelReduceHistogram<Ty> &o, tbb::split)
      : m_data{ o.m_data }
        , m_rawmin{ o.m_rawmin }
        , m_rawmax{ o.m_rawmax }
        , m_totalCount{ 0 }
  {
    m_buckets = new unsigned int[ NUM_BUCKETS ];
    m_histMin = new Ty[ NUM_BUCKETS ];
    m_histMax = new Ty[ NUM_BUCKETS ];

    for(int i{ 0 }; i < NUM_BUCKETS; ++i){
      m_buckets[i] = 0;
      m_histMin[i] = std::numeric_limits<Ty>::max();
      m_histMax[i] = std::numeric_limits<Ty>::lowest();
    }
  }

  ~ParallelReduceHistogram() { }

  void
  operator()(tbb::blocked_range<size_t> const &r)
  {
    unsigned int * const buckets{ m_buckets };
    Ty const * const data{ m_data };
    double const rawmin{ static_cast<double>(m_rawmin) };
    double const rawmax{ static_cast<double>(m_rawmax) };
    Ty * const histmin{ m_histMin };
    Ty * const histmax{ m_histMax };
    long long totalCount{ m_totalCount };


    for(size_t i{ r.begin() }; i != r.end(); ++i) {
      Ty val{ data[i] };

      // Compute the bucket index.
      long long idx{
          static_cast<long long>((val - rawmin)/(rawmax - rawmin) * MAX_IDX + 0.5) };

      // Reset idx if it is out of range.
//      if (idx < 0)
//        idx = 0;
      if (idx > MAX_IDX)
        idx = MAX_IDX;

      if (val < histmin[idx]) histmin[idx] = val;
      if (val > histmax[idx]) histmax[idx] = val;

      // Accumulate frequency.
      buckets[idx] += 1;
      totalCount += 1;
    }

    m_totalCount = totalCount;
  }

  void
  join(ParallelReduceHistogram<Ty> const &rhs)
  {
    // Accumulate histogram frequences from the joinee.
    for(size_t i{ 0 }; i < NUM_BUCKETS; ++i) {
      m_buckets[i] += rhs.m_buckets[i];

      if (rhs.m_histMin[i] < m_histMin[i])
        m_histMin[i] = rhs.m_histMin[i];

      if (rhs.m_histMax[i] > m_histMax[i])
        m_histMax[i] = rhs.m_histMax[i];
    }
    m_totalCount += rhs.m_totalCount;
  }

  unsigned int const * getBuckets() const { return m_buckets; }
  Ty const * getHistMin() const { return m_histMin; }
  Ty const * getHistMax() const { return m_histMax; }
  long long getTotalCount() const { return m_totalCount; }

private:
  Ty const * const m_data;
  Ty const m_rawmin;
  Ty const m_rawmax;
  long long m_totalCount;
  unsigned int * m_buckets; // [NUM_BUCKETS];
  Ty * m_histMin;
  Ty * m_histMax;
};

} // namespace preproc

#endif // ! preproc_parallelhistogram_h__
