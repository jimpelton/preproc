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
//#include <bd/io/fileblockcollection.h>

#include <vector>

namespace preproc
{

void
processRelMap(CommandLineOptions const &clo,
              bd::Volume &volume,
              std::vector<bd::FileBlock> & blocks);


/// \brief Use RMap to classify voxels as empty of non-empty within each block.
void
parallelCountBlockEmptyVoxels(CommandLineOptions const &clo,
                              bd::Volume &volume,
                              bd::Buffer<double> const *buf,
                              std::vector <bd::FileBlock> &blocks);

void
parallelSumBlockRelevances(CommandLineOptions const &clo,
                             bd::Volume &volume,
                             bd::Buffer<double> const *buf,
                             std::vector <bd::FileBlock> &blocks);



} // namespace preproc

#endif //PREPROCESSOR_PROCESSRELMAP_H
