//
// Created by jim on 10/15/16.
//

#ifndef PREPROCESSOR_PROCESSRELMAP_H
#define PREPROCESSOR_PROCESSRELMAP_H

#include "cmdline.h"

#include <bd/volume/volume.h>
#include <bd/io/buffer.h>
#include <bd/io/fileblock.h>

#include <vector>

namespace preproc
{


/// \brief For each buffer in the RMap file, count the number of irrelevant voxels for each
/// blocks, then compute the rov.
/// \param clo[in] clo - User supplied options.
/// \param volume[in,out] - The volume associated with the relevance map.
/// \param blocks[in,o
void
processRelMap(CommandLineOptions const &clo,
              bd::Volume &volume,
              std::vector<bd::FileBlock> & blocks);


///// \brief Classify the voxels as relevant or irrelevant depending on the
///// voxelOpacityRel_Min/Max command line options.
//void
//parallelCountBlockEmptyVoxels(CommandLineOptions const &clo,
//                              bd::Volume &volume,
//                              bd::Buffer<double> const *buf,
//                              std::vector <bd::FileBlock> &blocks);
//
///// \brief Sum the relevances in the RMap for each block.
//void
//parallelSumBlockRelevances(CommandLineOptions const &clo,
//                             bd::Volume &volume,
//                             bd::Buffer<double> const *buf,
//                             std::vector <bd::FileBlock> &blocks);

} // namespace preproc

#endif //PREPROCESSOR_PROCESSRELMAP_H
