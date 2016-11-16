//
// Created by jim on 10/15/16.
//

#include "processrelmap.h"
#include <bd/io/bufferedreader.h>
#include <bd/tbb/parallelreduce_blockempties.h>
#include <bd/tbb/parallelreduce_blockrov.h>

#include <tbb/tbb.h>

#include <stdexcept>

namespace preproc
{

namespace
{

/// \brief Use RMap to classify voxels as empty or non-empty within each block.
void
parallelCountBlockEmptyVoxels(CommandLineOptions const &clo,
                              bd::Volume &volume,
                              bd::Buffer<double> const *buf,
                              std::vector<bd::FileBlock> &blocks)
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
  tbb::blocked_range<size_t> range{ 0, buf->getNumElements() };
  tbb::parallel_reduce(range, empties);


  // Total the empty voxels for each block, and all the blocks.
  uint64_t const *emptyCounts{ empties.empties() };
  uint64_t totalEmpties{ 0 };
  for (size_t i{ 0 }; i < blocks.size(); ++i) {
    bd::FileBlock *b{ &blocks[i] };
    b->empty_voxels += emptyCounts[i];
    totalEmpties += emptyCounts[i];
  }

  volume.numEmptyVoxels(volume.numEmptyVoxels() + totalEmpties);

} // parallelCountBlockEmptyVoxels()

void
parallelSumBlockRelevances(CommandLineOptions const &clo,
                           bd::Volume &volume,
                           bd::Buffer<double> const *buf,
                           std::vector<bd::FileBlock> &blocks)
{

  bd::ParallelReduceBlockRov rov{ buf, &volume };
  tbb::blocked_range<size_t> range{ 0, buf->getNumElements() };
  tbb::parallel_reduce(range, rov);

  double const *vis{ rov.relevances() };
  for (size_t i{ 0 }; i < blocks.size(); ++i) {
    bd::FileBlock *b{ &blocks[i] };
    b->rov += vis[i];
  }

} // parallelSumBlockRelevances()

} // namespace


void
processRelMap(CommandLineOptions const &clo,
              bd::Volume &volume,
              std::vector<bd::FileBlock> &blocks)
{
  bd::BufferedReader<double> r{ clo.bufferSize };

  if (!r.open(clo.rmapFilePath)) {
    throw std::runtime_error("Could not open file: " + clo.rmapFilePath);
  }
  r.start();


  // In parallel compute block statistics based on the RMap values.
  bd::Buffer<double> *buf{ nullptr };
  while (( buf = r.waitNextFullUntilNone()) != nullptr) {
    parallelCountBlockEmptyVoxels(clo, volume, buf, blocks);
    parallelSumBlockRelevances(clo, volume, buf, blocks);

    r.waitReturnEmpty(buf);
  }


  // mark blocks as empty or non-empty by computing the ratio of visibility.
  for (auto &b : blocks) {
    if (b.rov >= clo.blockThreshold_Min && b.rov <= clo.blockThreshold_Max) {
      b.is_empty = 0;
    } else {
      b.is_empty = 1;
    }
  }


}


} // namespace preproc
