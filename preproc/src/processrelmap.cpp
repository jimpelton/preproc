//
// Created by jim on 10/15/16.
//

#include "processrelmap.h"

namespace preproc
{

void
parallelCountBlockEmptyVoxels(CommandLineOptions const &clo,
                              bd::Volume &volume,
                              bd::Buffer<double> const *buf,
                              std::vector <bd::FileBlock> &blocks)
{
  // if the relevance value from the rmap is in
  // [voxelOpacityRel_Min .. voxelOpacityRel_Max] then it is relevant.
  auto relevanceFunction = [&](double x) -> bool {
    return x >= clo.voxelOpacityRel_Min && x <= clo.voxelOpacityRel_Max;
  };


  // parallel_reduce body
  bd::ParallelReduceBlockEmpties<double, decltype(relevanceFunction)>
      empties{ buf, &volume, relevanceFunction };


  // count the voxels in parallel
  tbb::blocked_range <size_t> range{ 0, buf->getNumElements() };
  tbb::parallel_reduce(range, empties);

  uint64_t const *emptyCounts{ empties.empties() };

  //Total the empty voxels for each block, and all the blocks.
  uint64_t totalEmpties{ 0 };
  for (size_t i{ 0 }; i < blocks.size(); ++i) {
    bd::FileBlock *b{ &blocks[i] };
    b->empty_voxels += emptyCounts[i];
    totalEmpties += emptyCounts[i];
  }

  volume.numEmptyVoxels(volume.numEmptyVoxels() + totalEmpties);

//  Info() << "Done counting empties. \n\t" << totalEmpties << " empty voxels found.";

}

} // namespace preproc
