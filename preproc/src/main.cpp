////////////////////////////////////////////////////////////////////////////////
// Preprocessor
// Generates index files for simple_blocks viewer.
// Index file format
////////////////////////////////////////////////////////////////////////////////

#include "cmdline.h"
#include "volumeminmax.h"
#include "processrawfile.h"
#include "processrelmap.h"

#include <bd/util/util.h>
#include <bd/io/indexfile.h>
#include <bd/log/logger.h>
#include <bd/io/datfile.h>

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
              /*<< '_' << clo.blockThreshold_Min << '-' << clo.blockThreshold_Max*/
              << extension;

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


/// \brief Generate the IndexFile!
/// \throws std::runtime_error if rawfile can't be opened.
template<class Ty>
void
generateIndexFile(const CommandLineOptions &clo, bd::DataType type)
{
//  bd::FileBlockCollection<Ty>
//      collection{{ clo.vol_dims[0], clo.vol_dims[1], clo.vol_dims[2] },
//                 { clo.num_blks[0], clo.num_blks[1], clo.num_blks[2] }};



  bd::Info() << "Computing volume min/max.";
  double vMin, vMax, vTotal;
  volumeMinMax<Ty>(clo.inFile, clo.bufferSize, &vMin, &vMax, &vTotal);

  std::unique_ptr<bd::IndexFile> indexFile{ new bd::IndexFile() };
  indexFile->getVolume().voxelDims({ clo.vol_dims[0], clo.vol_dims[1], clo.vol_dims[2] });
  indexFile->getVolume().block_count({ clo.num_blks[0], clo.num_blks[1], clo.num_blks[2] });
  indexFile->getVolume().min(vMin);
  indexFile->getVolume().max(vMax);
  indexFile->getVolume().total(vTotal);
  indexFile->getVolume().avg(vTotal /
                                 double(clo.vol_dims[0] * clo.vol_dims[1] * clo.vol_dims[2]));
  indexFile->init(type);

  bd::Info() << "Processing raw file.";
  processRawFile<Ty>(clo,
                     indexFile->getVolume(),
                     indexFile->getFileBlocks(),
                     clo.skipRmapGeneration);

  if (!clo.skipRmapGeneration) {
    bd::Info() << "Processing relevance map.";
    processRelMap(clo,
                  indexFile->getVolume(),
                  indexFile->getFileBlocks());
  }

  writeIndexFileToDisk(*( indexFile.get()), clo);

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
    //std::cout << "\n---Begin Dat File---\n" << datfile << "\n---End Dat File---\n";

  }

  // Decide what data type we have and call execute to kick off the processing.
  bd::DataType type{ bd::to_dataType(clo.dataType) };

  switch (type) {

    case bd::DataType::UnsignedCharacter:
      preproc::generateIndexFile<unsigned char>(clo, type);
      break;

    case bd::DataType::UnsignedShort:
      preproc::generateIndexFile<unsigned short>(clo, type);
      break;

    case bd::DataType::Float:
      preproc::generateIndexFile<float>(clo, type);
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
      bd::IndexFile::fromBinaryIndexFile(clo.inFile) };

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
  if (numArgs == 0) {
    Err() << "Command line parse error, exiting.";
    bd::logger::shutdown();
    return 1;
  }
  //preproc::printThem(clo);

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

