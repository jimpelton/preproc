#ifndef blockaveragefilter_h__
#define blockaveragefilter_h__

#include "fileblock.h"


namespace preproc
{
class BlockAverageFilter
{

public:
  BlockAverageFilter(double tmin, double tmax) 
    : m_tmin{ tmin } 
    , m_tmax{ tmax } 
  { }

  ~BlockAverageFilter() { }

  bool operator()(const FileBlock &b)
  {
    return b.avg_val > m_tmin && b.avg_val < m_tmax;
  }


private:
  double m_tmin;
  double m_tmax;

};

} // namespace preproc

#endif // ! blockaveragefilter_h__
