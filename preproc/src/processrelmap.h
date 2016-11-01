//
// Created by jim on 10/15/16.
//

#ifndef PREPROCESSOR_PROCESSRELMAP_H
#define PREPROCESSOR_PROCESSRELMAP_H

#include "cmdline.h"

#include <bd/volume/volume.h>
#include <bd/io/bufferedreader.h>
#include <bd/io/buffer.h>
#include <bd/io/fileblock.h>
#include <bd/io/fileblockcollection.h>

#include <vector>

namespace preproc
{


/// \brief Use RMap to classify voxels as empty of non-empty within each block.
void
parallelCountBlockEmptyVoxels(CommandLineOptions const &clo,
                              bd::Volume &volume,
                              bd::Buffer<double> const *buf,
                              std::vector <bd::FileBlock> &blocks);

void
parallelSumBlockVisibilities(CommandLineOptions const &clo,
                             bd::Volume &volume,
                             bd::Buffer<double> const *buf,
                             std::vector <bd::FileBlock> &blocks);

/// \brief For each buffer in the RMap file, count the number of irrelevant voxels for each
/// blocks, then compute the rov.
/// \param clo
/// \param collection
template<class Ty>
void
processRelMap(CommandLineOptions const &clo,
              bd::FileBlockCollection <Ty> &collection)
{
  bd::BufferedReader<double> r{ clo.bufferSize };

  if (!r.open(clo.rmapFilePath)) {
    throw std::runtime_error("Could not open file: " + clo.rmapFilePath);
  }
  r.start();


  // In parallel compute block statistics based on the RMap values.
  bd::Buffer<double> *buf{ nullptr };
  while ((buf = r.waitNextFullUntilNone()) != nullptr) {
    parallelCountBlockEmptyVoxels(clo, collection.volume(), buf, collection.blocks());
    parallelSumBlockVisibilities(clo, collection.volume(), buf, collection.blocks());

    r.waitReturnEmpty(buf);
  }



  // compute the block ratio-of-visibility
//  for (auto &b : collection.blocks()) {
//    uint64_t totalvox{ b.voxel_dims[0] * b.voxel_dims[1] * b.voxel_dims[2] };
//    assert(totalvox > 0);
//    b.rov /= double(totalvox); //double(b.empty_voxels);
//  }

  // mark blocks as empty or non-empty by computing the ratio of visibility.
  for (auto &b : collection.blocks()) {
    if (b.rov >= clo.blockThreshold_Min && b.rov <= clo.blockThreshold_Max) {
      b.is_empty = 0;
    } else {
      b.is_empty = 1;
    }
  }


}


} // namespace preproc

#endif //PREPROCESSOR_PROCESSRELMAP_H
