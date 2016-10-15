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
#include <bd/volume/fileblockcollection.h>

#include <vector>

namespace preproc
{


/// \brief Use RMap to count the empty voxels in each block
void
parallelCountBlockEmptyVoxels(CommandLineOptions const &clo,
                              bd::Volume &volume,
                              bd::Buffer<double> const *buf,
                              std::vector <bd::FileBlock> &blocks);


/// For each buffer in the RMap file, count the number of irrelevant voxels for each
/// blocks, then compute
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

  while (r.hasNextBuffer()) {
    bd::Buffer<double> *buf{ r.waitNextFull() };
    parallelCountBlockEmptyVoxels(clo, collection.volume(), buf, collection.blocks());
    r.waitReturnEmpty(buf);
  }

  // compute the block ratio-of-visibility
  for (size_t i{ 0 }; i < collection.blocks().size(); ++i) {
    bd::FileBlock &b = collection.blocks()[i];
//    if (b.empty_voxels == 0) {
//      b.rov = 1.0;
//    } else {
    uint64_t totalvox{ b.voxel_dims[0] * b.voxel_dims[1] * b.voxel_dims[2] };
    uint64_t nonempty{ totalvox - b.empty_voxels };
    b.rov = nonempty / double(totalvox); //double(b.empty_voxels);
//    }

  }

  // mark blocks as empty or non-empty
  for (auto &b : collection.blocks()) {
    if (b.rov <= clo.blockThreshold_Min || b.rov >= clo.blockThreshold_Max) {
      b.is_empty = 1;
    }
  }
}


} // namespace preproc

#endif //PREPROCESSOR_PROCESSRELMAP_H
