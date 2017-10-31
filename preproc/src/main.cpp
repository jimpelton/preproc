////////////////////////////////////////////////////////////////////////////////
// Preprocessor
// Generates index files for simple_blocks viewer.
// Index file format
////////////////////////////////////////////////////////////////////////////////

#include "cmdline.h"
#include "volumeminmax.h"
#include "processrawfile.h"
#include "processrelmap.h"
#include "outputer.h"

#include <bd/util/util.h>
#include <bd/io/indexfile.h>
#include <bd/log/logger.h>
#include <bd/io/datfile.h>

#include <tbb/tbb.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

using bd::Err;
using bd::Info;
namespace fs = boost::filesystem;

namespace preproc
{

////////////////////////////////////////////////////////////////////////////////
std::string
makeFileNameString(const CommandLineOptions &clo, std::tuple<int, int, int> nb)
{
  std::stringstream outFileName;
  outFileName << clo.outFileDirLocation << '/' << clo.outFilePrefix
    << '_' << std::get<0>(nb) << '-'
    << std::get<1>(nb) << '-'
    << std::get<2>(nb);

  return outFileName.str();
}


////////////////////////////////////////////////////////////////////////////////
void
printBlocksToStdOut(bd::IndexFile const &indexFile)
{
  std::cout << "{\n";
  for (auto &block : indexFile.getFileBlocks()) {
    std::cout << block << std::endl;
  }
  std::cout << "}\n";
}


////////////////////////////////////////////////////////////////////////////////
void
writeIndexFileToDisk(bd::IndexFile const &indexFile,
                     std::string const &nameWithoutExtension,
                     CommandLineOptions const &clo)
{

  {
    std::string outFileName{ nameWithoutExtension + ".json" };
    indexFile.writeAsciiIndexFile(outFileName);
  }

  {
    std::string outFileName{ nameWithoutExtension + ".bin" };
    indexFile.writeBinaryIndexFile(outFileName);
  }

}


/// \brief Generate the IndexFile!
/// \throws std::runtime_error if rawfile can't be opened.
template<class Ty>
void
generateIndexFile(const CommandLineOptions &clo,
                  std::vector<std::tuple<int, int, int>> tuples,
                  bd::DataType type)
{


  fs::path rawPath(clo.inFile);
  fs::path tfPath(clo.tfuncPath);

  int numThreads = clo.numThreads;
  if (numThreads == 0) {
    numThreads = tbb::task_scheduler_init::default_num_threads();
  }
  tbb::task_scheduler_init init(numThreads);

  bd::Info() << "Computing volume min/max.";
  bd::Volume minmax{ {clo.vol_dims[0], clo.vol_dims[1], clo.vol_dims[2]}, {1, 1, 1} };
  volumeMinMax<Ty>(clo.inFile, clo.bufferSize, minmax);

  bool skipRmap{ clo.skipRmapGeneration };
  for (auto &t : tuples) {
    std::unique_ptr<bd::IndexFile> indexFile{ new bd::IndexFile() };
//    indexFile->setVolume(minmax);
    indexFile->getVolume().voxelDims(minmax.voxelDims());
    indexFile->getVolume().block_count({ std::get<0>(t), std::get<1>(t), std::get<2>(t) });
    indexFile->getVolume().min(minmax.min());
    indexFile->getVolume().max(minmax.max());
    indexFile->getVolume().avg(minmax.avg());
    indexFile->getVolume().total(minmax.total());
    indexFile->setRawFileName(rawPath.filename().string());
    indexFile->setTFFileName(tfPath.filename().string());
    indexFile->init(type);

    bd::Info() << "Processing raw file.";
    RFProc<Ty> proc;
    int const result{ 
      proc.processRawFile(clo, indexFile->getVolume(), indexFile->getFileBlocks(), skipRmap) };
    
    if (result != 0) {
      throw std::runtime_error("Problem processing raw file.");
    }

    std::cout << "Processing relevance map.";
    //    bd::Info() << "Processing relevance map.";
    processRelMap(clo, indexFile->getVolume(), indexFile->getFileBlocks());

    writeIndexFileToDisk(*(indexFile.get()), makeFileNameString(clo, t), clo);

    // we only need to write the Rmap one time, but we will keep processing it
    // for each iteration where we have a different block count.
    skipRmap = true;

  } // for(auto &t...
}

bool
makeNumBlocksTuples(std::vector<std::tuple<int, int, int>> &tups,
                    std::vector<std::string> const &strs)
{
  if (strs.size() == 0) {
    tups.push_back(std::make_tuple(1, 1, 1));
    return true;
  }

  for (auto &s : strs) {
    std::vector<std::string> split;
    boost::split(split, s, boost::is_any_of("x,"), boost::token_compress_on);
    if (split.size() < 3) {
      bd::Err() << "Block dimensions tuple has less than three!";
      return false;
    }
    int x{ std::stoi(split[0]) };
    int y{ std::stoi(split[1]) };
    int z{ std::stoi(split[2]) };
    tups.push_back(std::make_tuple(x, y, z));
  }
  return true;
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

  }

  std::vector<std::tuple<int, int, int>> tuples;
  makeNumBlocksTuples(tuples, clo.numBlocks);

  // Decide what data type we have and call generateIndexFile() to kick off the processing.
  bd::DataType type{ bd::to_dataType(clo.dataType) };

  switch (type) {

  case bd::DataType::UnsignedCharacter:
    preproc::generateIndexFile<unsigned char>(clo, tuples, type);
    break;

  case bd::DataType::UnsignedShort:
    preproc::generateIndexFile<unsigned short>(clo, tuples, type);
    break;

  case bd::DataType::Float:
    preproc::generateIndexFile<float>(clo, tuples, type);
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

  bool success;
  std::unique_ptr<bd::IndexFile> index{
      bd::IndexFile::fromBinaryIndexFile(clo.inFile, success) };

  if (clo.printBlocks) {

    // Print blocks in json format to standard out.
    index->writeAsciiIndexFile(std::cout);

  }
  else {

    // Otherwise, just write the blocks to a json text file.
    // We can't use makeFileNameString() because we want to use the binary file's name.

    auto startName = clo.inFile.rfind('/') + 1;
    auto endName = startName + (clo.inFile.size() - clo.inFile.rfind('.'));

    std::string name(clo.inFile, startName, endName);
    name += ".json";

    index->writeAsciiIndexFile(clo.outFileDirLocation + '/' + name);

  }
}

} // namespace preproc


///////////////////////////////////////////////////////////////////////////////
int
main(int argc, const char *argv[])
try
{
  preproc::Broker::start();
  using preproc::CommandLineOptions;
  CommandLineOptions clo;

  int numArgs = parseThem(argc, argv, clo);
  if (numArgs == 0) {
    Err() << "Command line parse error, exiting.";
    bd::logger::shutdown();
    return 1;
  }

  switch (clo.actionType) {

  case preproc::ActionType::Generate:
    preproc::generate(clo);
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

}
catch (std::exception &e) {

  Err() << "Caught exception in main: " << e.what();
  bd::logger::shutdown();
  return 1;

}

