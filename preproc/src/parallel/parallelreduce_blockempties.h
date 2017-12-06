#ifndef preproc_parallelblockstats_h
#define preproc_parallelblockstats_h

#include <bd/io/fileblock.h>
#include <bd/io/buffer.h>
#include <bd/volume/volume.h>


#include <tbb/blocked_range.h>

#include <functional>


namespace preproc
{

/// \brief Counts the number of empty voxels in each block.
/// Template parameter \c Function should be a callable type that
/// takes a Ty as a parameter and returns a bool. The callable type
/// should return true if the value it recieves is relevant, and
/// false if the value is not relevant.
template<class Ty, class Function>
class ParallelReduceBlockEmpties
{
public:

  ParallelReduceBlockEmpties(bd::Buffer<Ty> const *b, bd::Volume const *v, Function isRelevant)
    : m_data{ b->getPtr() }
    , m_volume{ v }
    , m_voxelStart{ b->getIndexOffset() }
    , m_empties{ nullptr }
    , isRelevant{ isRelevant }
  {
    m_empties = new uint64_t[m_volume->total_block_count()]();
  }

  ParallelReduceBlockEmpties(ParallelReduceBlockEmpties &o, tbb::split)
      : m_data{ o.m_data }
      , m_volume{ o.m_volume }
      , m_voxelStart{ o.m_voxelStart }
      , m_empties{ nullptr }
      , isRelevant{ o.isRelevant }
  {
    m_empties = new uint64_t[o.m_volume->total_block_count()]();
  }

  ~ParallelReduceBlockEmpties()
  {
    if (m_empties) {
      delete [] m_empties;
    }
  }

  void
  operator()(tbb::blocked_range<size_t> const &r)
  {
    Ty const * const a{ m_data };

    uint64_t const vdX{ m_volume->voxelDims().x };
    uint64_t const vdY{ m_volume->voxelDims().y };
    uint64_t const bdX{ m_volume->block_dims().x };
    uint64_t const bdY{ m_volume->block_dims().y };
    uint64_t const bdZ{ m_volume->block_dims().z };
    uint64_t const bcX{ m_volume->block_count().x };
    uint64_t const bcY{ m_volume->block_count().y };
    uint64_t const bcZ{ m_volume->block_count().z };
    uint64_t const voxelStart{ m_voxelStart };

    uint64_t vIdx, // voxel 1D index
        bI,        // block i index
        bJ,        // block j index
        bK,        // block k index
        bIdx;      // block 1D index

    for (size_t i{ r.begin() }; i != r.end(); ++i) {

      // Convert vIdx (the voxel index within the entire volume data set)
      // into a 3D index, then convert the 3D voxel index obtained from
      // vIdx into a 3D block index to be used in determining if our voxel
      // is within the part of the volume covered by our blocks.
      vIdx = i + voxelStart;
      bI = (vIdx % vdX) / bdX;
      bJ = ((vIdx / vdX) % vdY) / bdY;
      bK = ((vIdx / vdX) / vdY) / bdZ;

      if (bI < bcX && bJ < bcY && bK < bcZ) {
        Ty val = a[i];
        // Convert the 3D block index into a 1D block index and fetch the
        // block from the array of blocks.
        bIdx = bI + bcX * (bJ + bK * bcY);
        if (! isRelevant(val))
          m_empties[bIdx] += 1;
      }
    }
  }

  void
  join(ParallelReduceBlockEmpties const &rhs)
  {
    for(uint64_t i{ 0 }; i < m_volume->total_block_count(); ++i) {
      m_empties[i] += rhs.m_empties[i];
    }
  }


  uint64_t const *
  empties() const
  {
    return m_empties;
  }

private:
  Ty const * const m_data;
  bd::Volume const * const m_volume;
  size_t const m_voxelStart;
  uint64_t * m_empties;

  Function isRelevant; ///< Is the element a relevant voxel or not.

}; // class ParallelReduceBlockEmpties

} // namespace preproc

#endif // ! bd_parallelblockstats_h

