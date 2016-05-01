#ifndef blockaverage_h__
#define blockaverage_h__

#include "fileblock.h"
#include "volume.h"
#include "buffer.h"


#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>


namespace preproc
{

template<typename Ty>
class BlockAverage{

private:
  const Ty * const data;
  FileBlock** blocks;

  const uint64_t vdX, vdY;         // volume dims along X, Y axis
  const uint64_t bdX, bdY, bdZ;    // block dims along X, Y, Z axis
  const uint64_t bcX, bcY, bcZ;    // block count along X, Y, Z axis
  const uint64_t voxel_start;      // global voxel index this buffer starts at.

public:

  void operator()(const tbb::blocked_range<size_t> &r) const
  {
    const Ty * const a{ data };
    FileBlock** blks{ blocks };

    for (size_t i{ r.begin() }; i != r.end(); ++i) {
      // Convert idx (the voxel index within the entire volume data set)
      // into a 3D index.
      uint64_t idx{ i + voxel_start };
      uint64_t vX{ idx % vdX };
      uint64_t vY{ (idx / vdX) % vdY };
      uint64_t vZ{ (idx / vdX) / vdY };
      
      // Convert the 3D voxel index obtained from idx into a 3D block index
      // to be used in determining if our voxel is within the part of the
      // volume covered by our blocks.
      uint64_t bI{ vX/bdX };
      uint64_t bJ{ vY/bdY };
      uint64_t bK{ vZ/bdZ };

      if (bI < bcX && bJ < bcY && bK < bcZ) {
        Ty val = a[i];

        // Convert the 3D block index into a 1D block index and fetch the
        // block from the array of blocks.
        uint64_t blockIdx{ bI + bcX * (bJ + bK * bcY) };
        FileBlock *b = blks[blockIdx];

        // Accumulate block-specific values.
        if (val < b->min_val) { b->min_val = val; }
        if (val > b->max_val) { b->max_val = val; }
        b->total_val += val;
      }
    }

  }

  BlockAverage(const BlockAverage<Ty> &o) 
      : data{ o.data }
      , blocks{ o.blocks }
      , vdX{ o.vdX }
      , vdY{ o.vdY }
      , bdX{ o.bdX }
      , bdY{ o.bdY }
      , bdZ{ o.bdZ }
      , bcX{ o.bcX }
      , bcY{ o.bcY }
      , bcZ{ o.bcZ }
      , voxel_start{ o.voxel_start }
  { }

  BlockAverage(Buffer<Ty> *b, const Volume *v, FileBlock** blocks) 
      : data{ b->ptr() }
      , blocks{ blocks }
      , vdX{ v->dims().x }
      , vdY{ v->dims().y }
      , bdX{ v->lower().block_dims().x }
      , bdY{ v->lower().block_dims().y }
      , bdZ{ v->lower().block_dims().z }
      , bcX{ v->lower().block_count().x }
      , bcY{ v->lower().block_count().y }
      , bcZ{ v->lower().block_count().z }
      , voxel_start{ b->index() }
  { }

}; // class BlockAverage

} // namespace preproc

#endif // ! blockaverage_h__

