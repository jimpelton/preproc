//
// Created by Jim Pelton on 8/31/16.
//

#ifndef bd_parallelvoxelclassifier_h__
#define bd_parallelvoxelclassifier_h__

#include <bd/io/buffer.h>

#include <tbb/tbb.h>

#include <vector>
#include <functional>

namespace preproc
{

/// \brief Classifies individual voxels as relevant or irrelevant.
///        The classification is saved in the voxel relevance map.
template<class Ty, class Function, class Storage>
class ParallelForVoxelRelevance
{
public:

  ParallelForVoxelRelevance(Storage &map,
                          bd::Buffer<Ty> const *buf,
                          Function const &relevant)
      : m_map{ &map }
      , m_buf{ buf }
      , m_isRel{ relevant }
  {
  }

  ParallelForVoxelRelevance(ParallelForVoxelRelevance const &rhs, tbb::split)
      : m_map{ rhs.m_map }
      , m_buf{ rhs.m_buf }
      , m_isRel{ rhs.m_isRel }
  {
  }

  ~ParallelForVoxelRelevance()
  {
  }

  void operator()(tbb::blocked_range<size_t> const &r) const
  {
    Ty const * const data{ m_buf->getPtr() };

    for(size_t i{ r.begin() }; i != r.end(); ++i) {
      double val = m_isRel(data[i]);

      if (val != val || std::isnan(val) || std::isinf(val)) {
        std::cout << "Got a nan!\n";
//        DebugBreak();
      }

      (*m_map)[i] = val;
    }
  }

private:
  Storage * const m_map;  ///< Relevance map
  bd::Buffer<Ty> const * m_buf;
  Function const &m_isRel;

}; // class ParallelForVoxelClassifier

} // namespace preproc

#endif // ! bd_parallelvoxelclassifier_h__
