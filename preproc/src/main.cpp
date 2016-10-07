////////////////////////////////////////////////////////////////////////////////
// Preprocessor
// Generates index files for simple_blocks viewer.
// Index file format
////////////////////////////////////////////////////////////////////////////////

#include "cmdline.h"
#include "voxelopacityfunction.h"
#include "processrawfile.h"

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

using bd::Err;
using bd::Info;

namespace preproc
{


////////////////////////////////////////////////////////////////////////////////
std::string
makeFileNameString(const CommandLineOptions &clo, const char *extension)
{
  std::stringstream outFileName;
  outFileName << clo.outFilePath << '/' << clo.outFilePrefix
              << '_' << clo.num_blks[0] << '-' << clo.num_blks[1] << '-'
              << clo.num_blks[2]
              << '_' << clo.blockThreshold_Min << '-' << clo.blockThreshold_Max
              << extension;

  return outFileName.str();
}


////////////////////////////////////////////////////////////////////////////////
void
printBlocksToStdOut(bd::IndexFile const &indexFile)
{
  std::cout << "{\n";
  for (auto &block : indexFile.getBlocks()) {
    std::cout << block << std::endl;
  }
  std::cout << "}\n";
}


////////////////////////////////////////////////////////////////////////////////
void
writeIndexFileToDisk(bd::IndexFile const &indexFile, CommandLineOptions const &clo)
{

  {
    std::string outFileName{ makeFileNameString(clo, ".json") };
    indexFile.writeAsciiIndexFile(outFileName);
  }

  {
    std::string outFileName{ makeFileNameString(clo, ".bin") };
    indexFile.writeBinaryIndexFile(outFileName);
  }

}



/// \brief Use RMap to count the empty voxels in each block
void
parallelCountBlockEmptyVoxels(CommandLineOptions const &clo,
                              bd::Volume &volume,
                              bd::Buffer<double> const *buf,
                              std::vector<bd::FileBlock> &blocks)
{
  // relevance function
  auto rfunc = [&](double x) -> bool
  {
    return x >= clo.voxelOpacityRel_Min && x <= clo.voxelOpacityRel_Max;
  };

  bd::ParallelReduceBlockEmpties<double, decltype(rfunc)>
      empties{ buf, &volume, rfunc };

  tbb::blocked_range<size_t> range{ 0, buf->getNumElements() };
  tbb::parallel_reduce(range, empties);

  uint64_t const *emptyCounts{ empties.empties() };

  //Total the empty voxels for each block, and all the blocks.
  uint64_t totalEmpties{ 0 };
  for (size_t i{ 0 }; i < blocks.size(); ++i) {
    bd::FileBlock *b{ &blocks[i] };
    b->empty_voxels += emptyCounts[i];
    totalEmpties += emptyCounts[i];
  }

  volume.numEmptyVoxels( volume.numEmptyVoxels() + totalEmpties );

//  Info() << "Done counting empties. \n\t" << totalEmpties << " empty voxels found.";

}


/// For each buffer in the RMap file, count the number of irrelevant voxels for each
/// blocks, then compute
/// \param clo
/// \param collection
template<class Ty>
void
processRelMap(CommandLineOptions const &clo,
              bd::FileBlockCollection<Ty> &collection)
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

  for (size_t i{ 0 }; i < collection.blocks().size(); ++i) {
    bd::FileBlock &b = collection.blocks()[i];
    uint64_t blockVox{ b.voxel_dims[0] * b.voxel_dims[1] * b.voxel_dims[2] };
    b.rov = double(blockVox - b.empty_voxels) / double(b.empty_voxels);
  }
}

/// \brief Generate the IndexFile!
/// \throws std::runtime_error if rawfile can't be opened.
template<typename Ty>
void
generateIndexFile(const CommandLineOptions &clo)
{
  bd::FileBlockCollection<Ty>
      collection{{ clo.vol_dims[0], clo.vol_dims[1], clo.vol_dims[2] },
                 { clo.num_blks[0], clo.num_blks[1], clo.num_blks[2] }};

  collection.volume().min(clo.volMin);
  collection.volume().max(clo.volMax);

  processRawFile<Ty>(clo,
                     collection.volume(),
                     collection.blocks(),
                     clo.skipRmapGeneration);

  if (! clo.skipRmapGeneration) {
    processRelMap(clo, collection);
  }

  std::unique_ptr<bd::IndexFile>
      indexFile{
          bd::IndexFile::fromBlockCollection<Ty>(clo.inFile,
                                                 collection,
                                                 bd::to_dataType(clo.dataType)) };


  writeIndexFileToDisk(*(indexFile.get()), clo);

}

/// \throws std::runtime_error if rawfile can't be opened.
void
generate(CommandLineOptions &clo)
{
  // if a dat file was provided, populate our CommandLineOptions with the
  // options from that dat file.
  if (!clo.datFilePath.empty()) {

    bd::DatFileData datfile;
    bd::parseDat(clo.datFilePath, datfile);

    clo.vol_dims[0] = datfile.rX;
    clo.vol_dims[1] = datfile.rY;
    clo.vol_dims[2] = datfile.rZ;

    clo.dataType = bd::to_string(datfile.dataType);

    bd::Info() << clo << std::endl; // print cmd line options
    std::cout << "\n---Begin Dat File---\n" << datfile << "\n---End Dat File---\n";

  }

  // Decide what data type we have and call execute to kick off the processing.
  bd::DataType type{ bd::to_dataType(clo.dataType) };

  switch (type) {

    case bd::DataType::UnsignedCharacter:
      preproc::generateIndexFile<unsigned char>(clo);
      break;

    case bd::DataType::UnsignedShort:
      preproc::generateIndexFile<unsigned short>(clo);
      break;

    case bd::DataType::Float:
      preproc::generateIndexFile<float>(clo);
      break;

    default:
      bd::Err() << "Unsupported/unknown datatype: " << clo.dataType << ".\n Exiting...";
      break;

  }
}


/// \brief Open a binary index file and print it to stdout or write it to json file.
void
convert(CommandLineOptions &clo)
{

  std::unique_ptr<bd::IndexFile> index{
      bd::IndexFile::fromBinaryIndexFile(clo.inFile)
  };

  if (clo.printBlocks) {

    // Print blocks in json format to standard out.
    index->writeAsciiIndexFile(std::cout);

  } else {

    // Otherwise, just write the blocks to a json text file.
    // We can't use makeFileNameString() because we want to use the binary file's name.

    auto startName = clo.inFile.rfind('/') + 1;
    auto endName = startName + ( clo.inFile.size() - clo.inFile.rfind('.'));

    std::string name(clo.inFile, startName, endName);
    name += ".json";

    index->writeAsciiIndexFile(clo.outFilePath + '/' + name);

  }
}

} // namespace preproc


///////////////////////////////////////////////////////////////////////////////
int
main(int argc, const char *argv[])
try
{

  using preproc::CommandLineOptions;
  CommandLineOptions clo;

  int numArgs = parseThem(argc, argv, clo);
  preproc::printThem(clo);
  if (numArgs == 0) {
    Err() << "Command line parse error, exiting.";
    bd::logger::shutdown();
    return 1;
  }

  switch (clo.actionType) {

    case preproc::ActionType::Generate:

      try {
        preproc::generate(clo);
      }
      catch (std::exception e) {
        bd::Err() << e.what();
      }
      break;

    case preproc::ActionType::Convert:
      preproc::convert(clo);
      break;

    default:
      Err() << "Provide an action. Use -h for help.";
      bd::logger::shutdown();
      return 1;
  }

  bd::logger::shutdown();

  return 0;

} catch (std::exception e) {

  Err() << "Caught exception in main: " << e.what();
  bd::logger::shutdown();
  return 1;

}

