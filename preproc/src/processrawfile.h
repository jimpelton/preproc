//
// Created by Jim Pelton on 10/3/16.
//


#include "cmdline.h"
#include "voxelopacityfunction.h"

#include <bd/volume/fileblockcollection.h>
#include <bd/util/util.h>
#include <bd/io/indexfile.h>
#include <bd/log/logger.h>
#include <bd/io/datfile.h>
#include <bd/filter/valuerangefilter.h>
#include <bd/tbb/parallelfor_voxelclassifier.h>
#include <bd/volume/transferfunction.h>

#include <tbb/tbb.h>

#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

namespace preproc
{


template <class Ty>
void
parallelBlockMinMax(bd::Volume const &volume,
                    std::vector<bd::FileBlock> &blocks,
                    bd::Buffer<Ty> const *buf)
{
  bd::ParallelReduceBlockMinMax<Ty> minMax{ &volume, buf };
  tbb::blocked_range<size_t> range{ 0, buf->getNumElements() };
  tbb::parallel_reduce(range, minMax);

  bd::MinMaxPairDouble const *pairs{ minMax.pairs() };
  for(size_t i{ 0 }; i < blocks.size(); ++i){
    bd::FileBlock &b = blocks[i];

    if (b.min_val > pairs[i].min){
      b.min_val = pairs[i].min;
    }

    if(b.max_val < pairs[i].max){
      b.max_val = pairs[i].max;
    }

    b.total_val += pairs[i].total;
  }

} // parallelBlockMinMax

template<class Ty>
void
createRelMap(bd::Buffer<Ty> const *buf,
             preproc::VoxelOpacityFunction<Ty> &relFunc,
             std::vector<double> &relMap,
             std::ofstream &rmapfile)
{

  using Classifier =
  bd::ParallelForVoxelClassifier<Ty,
                                 preproc::VoxelOpacityFunction<Ty>,
                                 std::vector<double>>;

  // The voxel classifier uses the opacity function to write the
  // opacity to the rmap.
  Classifier classifier{ relMap, buf, relFunc };

  // Process this buffer in parallel with the classifier
  tbb::blocked_range <size_t> range{ 0, buf->getNumElements() };
  tbb::parallel_for(range, classifier);

  // Write RMap for this buffer out to disk.
  rmapfile.write(reinterpret_cast<char *>(relMap.data()),
                 buf->getNumElements() * sizeof(double));

} // createRelMap




/// \brief Create the relevance map in parallel based on the transfer function.
/// \param clo The command line options
/// \param tempOutFilePath Path that the temp RMap scratch file should be written to.
/// \throws std::runtime_error If the raw file could not be opened.
template<class Ty>
int
processRawFile(CommandLineOptions const &clo,
               bd::Volume const &volume,
               std::vector<bd::FileBlock> &blocks)
{

  try {
    // Generate the transfer function and set up
    // the VoxelOpacityFunction
    bd::OpacityTransferFunction trFunc{ clo.tfuncPath };
    if (trFunc.getNumKnots() == 0) {
      bd::Err() << "Transfer function has size 0.";
      return -1;
    }
    preproc::VoxelOpacityFunction<Ty> relFunc{ trFunc, clo.volMin, clo.volMax };

    // Open the raw file in a BufferedReader.
    bd::BufferedReader<Ty> r{ clo.bufferSize };
    if (!r.open(clo.inFile)) {
      bd::Err() << "Could not open file " + clo.inFile;
      return -1;
    }
    r.start();

    // Open the output rmap file.
    std::ofstream rmapfile{ clo.rmapFilePath };
    if (!rmapfile.is_open()) {
      bd::Err() << "Could not open rmap output file: " << clo.rmapFilePath;
      return -1;
    }

    // The relevance map is stored in pre-allocated vector.
    // It is only as large as a single buffer from the reader
    // and is flushed to disk after each buffer is analyzed.
    std::vector<double> relMapBuffer(r.singleBufferElements(), 0);

    while (r.hasNextBuffer()) {

      bd::Buffer <Ty> *b{ r.waitNextFull() };

      createRelMap(b, relFunc, relMapBuffer, rmapfile);
      parallelBlockMinMax(volume, blocks, b);

      r.waitReturnEmpty(b);

    }

    rmapfile.close();
  }
  catch (std::exception e) {
    bd::Err() << e.what();
    return -1;
  }

  return 0;

} // processRawFile()

} // namespace preproc