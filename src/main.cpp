////////////////////////////////////////////////////////////////////////////////
// Preprocessor
// Generates index files for simple_blocks viewer.
// Index file format
////////////////////////////////////////////////////////////////////////////////

#include "cmdline.h"

#include <bd/util/util.h>
#include <bd/io/indexfile.h>
#include <bd/volume/blockcollection2.h>
#include <bd/log/logger.h>
#include <bd/io/parsedat.h>

#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>

using bd::Err;
using bd::Info;

namespace preproc {


std::string
makeFileNameString(const CommandLineOptions &clo, const char *extension)
{
  std::stringstream outFileName;
  outFileName << clo.outFilePath << '/' << clo.outFilePrefix << '_' <<
      clo.num_blks[0] << '-' << clo.num_blks[1] << '-' << clo.num_blks[2] << '_' <<
      clo.tmin << '-' << clo.tmax << extension;

  return outFileName.str();
}

template<typename Ty>
void
generateIndexFile(const CommandLineOptions &clo)
{

  float minmax[2];
  minmax[0] = clo.tmin;
  minmax[1] = clo.tmax;

  try {
    bd::IndexFile * indexFile{
        bd::IndexFile::fromRawFile(
            clo.inFile,
            clo.bufferSize,
            bd::to_dataType(clo.dataType),
            clo.vol_dims,
            clo.num_blks,
            minmax) };

    switch (clo.outputFileType) {

    case OutputType::Ascii: {
      std::string outFileName{ makeFileNameString(clo, ".json") };
      indexFile->writeAsciiIndexFile(outFileName);
      break;
    }

    case OutputType::Binary: {
      std::string outFileName{ makeFileNameString(clo, ".bin") };
      indexFile->writeBinaryIndexFile(outFileName);
      break;
    }

    } //switch

    if (clo.printBlocks) {
      std::stringstream ss;
      ss << "{\n";
      for (bd::FileBlock *block : indexFile->blocks()) {
        std::cout << *block << std::endl;
      }
      ss << "}\n";
    }
  } catch (std::runtime_error e) {
    //std::cerr << e.what() << std::endl;
    Err() << e.what();
  }
}

template<typename Ty>
void
readIndexFile(const CommandLineOptions & clo)
{

  bd::IndexFile * index{
      bd::IndexFile::fromBinaryIndexFile(clo.inFile)
  };

  auto startName = clo.inFile.rfind('/')+1;
  auto endName = startName + (clo.inFile.size() - clo.inFile.rfind('.'));
  std::string name(clo.inFile, startName, endName);
  name += ".json";

  index->writeAsciiIndexFile(clo.outFilePath + '/' + name);
}

template<typename Ty>
void
execute(const CommandLineOptions &clo)
{
  if (clo.actionType == ActionType::Generate) {
    generateIndexFile<Ty>(clo);
  } else {
    readIndexFile<Ty>(clo);
  }
}

} // namespace preproc

///////////////////////////////////////////////////////////////////////////////
int
main(int argc, const char *argv[])
{

  Info() << "Test: " << 5;
  using preproc::CommandLineOptions;
  CommandLineOptions clo;
  if (parseThem(argc, argv, clo) == 0) {
    Err() << "Command line parse error, exiting.";
    bd::logger::shutdown();
    return 1;
  }


  if (clo.actionType == preproc::ActionType::Generate) {
    if (!clo.datFilePath.empty()) {
      bd::DatFileData datfile;
      bd::parseDat(clo.datFilePath, datfile);
      clo.vol_dims[0] = datfile.rX;
      clo.vol_dims[1] = datfile.rY;
      clo.vol_dims[2] = datfile.rZ;
      clo.dataType = bd::to_string(datfile.dataType);
      std::cout << ".dat file: \n" << datfile << "\n.";
    }
  }

  std::cout << clo << std::endl; // print cmd line options

  bd::DataType type{ bd::to_dataType(clo.dataType) };
  switch (type) {

  case bd::DataType::UnsignedCharacter:
    preproc::execute<unsigned char>(clo);
    break;

  case bd::DataType::UnsignedShort:
    preproc::execute<unsigned short>(clo);
    break;

  case bd::DataType::Float:
    preproc::execute<float>(clo);
    break;

  default:
    std::cerr << "Unsupported/unknown datatype: " << clo.dataType << ".\n Exiting...";
    return 1;
    break;
  }

  bd::logger::shutdown();

  return 0;
}

