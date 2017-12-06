//
// Created by jim on 10/18/16.
//

#ifndef preproc_parallelreduce_blockrov_h
#define preproc_parallelreduce_blockrov_h


#include <bd/io/fileblock.h>
#include <bd/io/buffer.h>
#include <bd/volume/volume.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>

#include <functional>


namespace preproc
{

/// \brief Sums the relevance values in each block.
///
/// Template parameter \c Function should be a callable type that
/// takes a Ty as a parameter and returns a bool. The callable type
/// should return true if the value it recieves is relevant, and
/// false if the value is not relevant.
class ParallelReduceBlockRov
{
public:

  ParallelReduceBlockRov(bd::Buffer<double> const *b, bd::Volume const *v)
      : m_data{ b->getPtr() }
      , m_volume{ v }
      , m_voxelStart{ b->getIndexOffset() }
      , m_rels{ nullptr }
//      , alpha{ alpha }
  {
    m_rels = new double[m_volume->total_block_count()]();
  }

  ParallelReduceBlockRov(ParallelReduceBlockRov &o, tbb::split)
      : m_data{ o.m_data }
      , m_volume{ o.m_volume }
      , m_voxelStart{ o.m_voxelStart }
      , m_rels{ nullptr }
//      , alpha{ o.alpha }
  {
    m_rels = new double[o.m_volume->total_block_count()]();
  }

  ~ParallelReduceBlockRov()
  {
    if (m_rels) {
      delete [] m_rels;
    }
  }

  void
  operator()(tbb::blocked_range<size_t> const &r)
  {
    double const * const a{ m_data };

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
        double val = a[i];
        // Convert the 3D block index into a 1D block index and fetch the
        // block from the array of blocks.
        bIdx = bI + bcX * (bJ + bK * bcY);
        m_rels[bIdx] += val;
      }
    }
  }

  void
  join(ParallelReduceBlockRov const &rhs)
  {
    for(uint64_t i{ 0 }; i < m_volume->total_block_count(); ++i) {
      m_rels[i] += rhs.m_rels[i];
    }
  }


  double const *
  relevances() const
  {
    return m_rels;
  }

private:
  double const * const m_data;
  bd::Volume const * const m_volume;
  size_t const m_voxelStart; ///< Offset into the volume that this buffer starts at.
  double * m_rels;

//  Function alpha; ///< Is the element a relevant voxel or not.

}; // class ParallelReduceBlockRov

} // namespace preproc


#endif //! bd_parallelreduce_blockrov_h
