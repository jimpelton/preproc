//
// Created by jim on 10/15/16.
//

#include "processrelmap.h"
#include "parallel/parallelreduce_blockempties.h"
#include "parallel/parallelreduce_blockrov.h"

#include <bd/io/bufferedreader.h>

#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

#include <stdexcept>


namespace preproc
{
namespace
{


/// \brief Use RMap to classify voxels as empty or non-empty within each block.
//void
//parallelCountBlockEmptyVoxels(bd::Buffer<double> const *buf,
//                              CommandLineOptions const &clo,
//                              bd::Volume &volume,
//                              std::vector<bd::FileBlock> &blocks)
//{
//  // if the relevance value from the rmap is in
//  // [voxelOpacityRel_Min .. voxelOpacityRel_Max] then it is relevant.
//  auto relevanceFunction =
//      [&](double x) -> bool
//      {
//        return x >= clo.voxelOpacityRel_Min && x <= clo.voxelOpacityRel_Max;
//      };
//
//  using Classifier = bd::ParallelReduceBlockEmpties<double, decltype(relevanceFunction)>;
//
//  Classifier empties{ buf, &volume, relevanceFunction };
//
//
//  // count the voxels in parallel
//  tbb::blocked_range<size_t> range{ 0, buf->getNumElements() };
//  tbb::parallel_reduce(range, empties);
//
//
//  // Total the empty voxels for each block.
//  uint64_t const *emptyCounts{ empties.empties() };
//  for (size_t i{ 0 }; i < blocks.size(); ++i) {
//    bd::FileBlock *b{ &blocks[i] };
//    b->empty_voxels += emptyCounts[i];
//  }
//
//} // parallelCountBlockEmptyVoxels()


///////////////////////////////////////////////////////////////////////////////
/// parallelSumBlockRelevances is executed once for each buffer. Each time its
/// executed it computes the ROV for the blocks associated with the buffer then
/// updates each block in the \c blocks vector.
/// 
/// \param buf the buffer to process
/// \param volume 
/// \param blocks
void
parallelSumBlockRelevances(bd::Buffer<double> const *buf,
                           bd::Volume &volume,
                           std::vector<bd::FileBlock> &blocks)
{

  ParallelReduceBlockRov rov{ buf, &volume };
  tbb::blocked_range<size_t> range{ 0, buf->getNumElements() };
  tbb::parallel_reduce(range, rov);

  double const *vis{ rov.relevances() };
  for (size_t i{ 0 }; i < blocks.size(); ++i) {
    bd::FileBlock *b{ &blocks[i] };
    b->rov += vis[i];
  }
} // parallelSumBlockRelevances()
} // namespace


/// \brief For each buffer in the RMap file, count the number of irrelevant voxels for each
/// blocks, then compute the rov.
/// \param clo - 
/// \param volume - 
/// \param blocks - 
void
processRelMap(CommandLineOptions const &clo,
              bd::Volume &volume,
              std::vector<bd::FileBlock> &blocks)
{
  bd::BufferedReader<double> r{ clo.bufferSize };

  bd::Info() << "Opening rmap file for processing: " << clo.rmapFilePath;
  if (!r.open(clo.rmapFilePath)) {
    throw std::runtime_error("Could not open file: " + clo.rmapFilePath);
  }
  r.start();

  // In parallel, compute block statistics based on the RMap values.
  // This loop runs for each buffer filled from the rmap file.
  bd::Buffer<double> *buf{ nullptr };
  while ((buf = r.waitNextFullUntilNone()) != nullptr) {

//    parallelCountBlockEmptyVoxels(buf, clo, volume, blocks);

    parallelSumBlockRelevances(buf, volume, blocks);

    r.waitReturnEmpty(buf);
  }


  // compute the block relevance as a ratio of
  for (auto &b : blocks) {
    uint64_t totalvox{ b.voxel_dims[0] * b.voxel_dims[1] * b.voxel_dims[2] };
    b.rov /= double(totalvox); //double(b.empty_voxels);
  }

//  std::for_each(blocks.begin(),
//                blocks.end(),
//                [&clo](bd::FileBlock &b) -> void {
//                  if (b.rov >= clo.blockThreshold_Min && b.rov <= clo.blockThreshold_Max) {
//                    b.is_empty = 0;
//                  } else {
//                    b.is_empty = 1;
//                  }
//                });

  auto minmaxE =
    std::minmax_element(blocks.begin(),
                        blocks.end(),
                        [](bd::FileBlock const &lhs, bd::FileBlock const &rhs) -> bool {
                          return lhs.rov < rhs.rov;
                        });

  volume.rovMin((*minmaxE.first).rov);
  volume.rovMax((*minmaxE.second).rov);

} // processRelMap()
} // namespace preproc
