////////////////////////////////////////////////////////////////////////////////
// Preprocessor
// Generates index files for simple_blocks viewer.
// Index file format
////////////////////////////////////////////////////////////////////////////////

#include "cmdline.h"
#include "voxelopacityfunction.h"

#include <bd/util/util.h>
#include <bd/io/indexfile.h>
#include <bd/log/logger.h>
#include <bd/io/datfile.h>
//#include <bd/filter/voxelopacityfilter.h>
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
              << '_' << clo.blockThresholdMin << '-' << clo.blockThresholdMax
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
                              std::vector<bd::FileBlock> &blocks)
{

  bd::BufferedReader<char> r{ clo.bufferSize };
  if (!r.open(clo.rmapFilePath)) {
    throw std::runtime_error("Could not open file: " + clo.rmapFilePath);
  }
  r.start();


  // relevance function
  auto rfunc = [](char x) -> bool { return x == 1; };

  uint64_t totalEmpties{ 0 };

  while (r.hasNext()) {

    bd::Buffer<char> *buf{ r.waitNext() };

    bd::ParallelReduceBlockEmpties<char, decltype(rfunc)>
        empties{ buf, &volume, rfunc };

    tbb::blocked_range<size_t> range{ 0, buf->getNumElements() };
    tbb::parallel_reduce(range, empties);

    uint64_t const *emptyCounts{ empties.empties() };
    for (size_t i{ 0 }; i < blocks.size(); ++i) {
      blocks[i].empty_voxels += emptyCounts[i];
      totalEmpties += emptyCounts[i];
    }

    r.waitReturn(buf);


  }


  //TODO: volume.totalempties(totalempties);
  Info() << "Done counting empties. \n\t" << totalEmpties << " empty voxels found.";

}


/// \brief Create the relevance map in parallel based on the transfer function.
/// \param clo The command line options
/// \param tempOutFilePath Path that the temp RMap scratch file should be written to.
/// \throws std::runtime_error If the raw file could not be opened.
template<typename Ty>
int
createRelMap(CommandLineOptions const &clo)
{

//  std::vector<bd::OpacityKnot> trFunc;

  try {
    bd::OpacityTransferFunction trFunc{ clo.tfuncPath };

    if (trFunc.getNumKnots() == 0) {
      bd::Err() << "Transfer function has size 0.";
      return -1;
    }

    preproc::VoxelOpacityFunction<Ty> relFunc{ trFunc, clo.volMin, clo.volMax };
//  bd::ValueRangeFunction<Ty> relFunc{ clo.blockThresholdMin, clo.blockThresholdMax };

    bd::BufferedReader<Ty> r{ clo.bufferSize };
    if (!r.open(clo.inFile)) {
      bd::Err() << "Could not open file " + clo.inFile;
      return -1;
    }
    r.start();

    std::ofstream rmapfile{ clo.rmapFilePath };
    if (!rmapfile.is_open()) {
      bd::Err() << "Could not open " << clo.rmapFilePath;
      return -1;
    }

//    size_t const bufSize{ r.singleBufferElements() * sizeof(Ty) };
    std::vector<double> relMap(r.singleBufferElements(), 0);

    while (r.hasNext()) {

      bd::Buffer<Ty> *b{ r.waitNext() };

      bd::ParallelForVoxelClassifier
          <Ty, preproc::VoxelOpacityFunction<Ty>, std::vector<double>>
          classifier{ relMap, b, relFunc };

      tbb::blocked_range<size_t> range{ 0, b->getNumElements() };
      tbb::parallel_for(range, classifier);


      r.waitReturn(b);


      // Write RMap for this buffer out to disk.
      rmapfile.write(reinterpret_cast<char *>(relMap.data()),
                     b->getNumElements() * sizeof(double));

    }

    rmapfile.close();

  }
  catch (std::exception e) {

    bd::Err() << e.what();
    return -1;

  }
  return 0;

}


/// \brief Generate the IndexFile!
/// \throws std::runtime_error if rawfile can't be opened.
template<typename Ty>
void
generateIndexFile(const CommandLineOptions &clo)
{
  createRelMap<Ty>(clo);


  // TODO: Octree decompostion of R-Map
  // TODO: Block generation

  bd::FileBlockCollection<Ty>
      collection{{ clo.vol_dims[0], clo.vol_dims[1], clo.vol_dims[2] },
                 { clo.num_blks[0], clo.num_blks[1], clo.num_blks[2] }};

  collection.volume().min(clo.volMin);
  collection.volume().max(clo.volMax);


//  parallelCountBlockEmptyVoxels(clo, collection.volume(), collection.blocks());


  // TODO: IndexFile generation
//  std::unique_ptr<bd::IndexFile>
//      indexFile{
//          bd::IndexFile::fromBlockCollection<Ty>(clo.inFile,
//                                                 collection,
//                                                 bd::to_dataType(clo.dataType)) };


  // TODO: Write IndexFile to disk.
//  writeIndexFileToDisk(*(indexFile.get()), clo);

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

  if (parseThem(argc, argv, clo) == 0) {
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

