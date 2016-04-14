////////////////////////////////////////////////////////////////////////////////
// Preprocessor
// Generates index files for simple_blocks viewer.
// Index file format
////////////////////////////////////////////////////////////////////////////////

#include "cmdline.h"
#include "util.h"

#include "indexfile.h"
#include "blockcollection2.h"
#include "logger.h"
#include "parsedat.h"

#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>

using preproc::Err;
using preproc::Info;

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
  //const size_t bufsz = 67'108'864;
  try {
    std::shared_ptr<preproc::IndexFile> indexFile{
        preproc::IndexFile::fromRawFile(
            clo.inFile,
            clo.bufferSize,
            preproc::to_dataType(clo.dataType),
            clo.vol_dims,
            clo.num_blks,
            minmax)
    };

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
    }

    if (clo.printBlocks) {
      std::stringstream ss;
      ss << "{\n";
      for (preproc::FileBlock *block : indexFile->blocks()) {
        std::cout << *block << std::endl;
      }
      ss << "}\n";
    }
  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl;
    Err() << e.what();
  }
}

template<typename Ty>
void
readIndexFile(const CommandLineOptions & clo)
{

  std::shared_ptr<preproc::IndexFile> index{
      preproc::IndexFile::fromBinaryIndexFile(clo.inFile)
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


///////////////////////////////////////////////////////////////////////////////
int
main(int argc, const char *argv[])
{

  CommandLineOptions clo;
  if (parseThem(argc, argv, clo) == 0) {
    Err() << "Command line parse error, exiting.";
    preproc::logger::shutdown();
    exit(1);
  }

  if (clo.actionType == ActionType::Generate) {
    if (!clo.datFilePath.empty()) {
      preproc::DatFileData datfile;
      preproc::parseDat(clo.datFilePath, datfile);
      clo.vol_dims[0] = datfile.rX;
      clo.vol_dims[1] = datfile.rY;
      clo.vol_dims[2] = datfile.rZ;
      clo.dataType = preproc::to_string(datfile.dataType);
      std::cout << ".dat file: \n" << datfile << "\n.";
    }
  }

  std::cout << clo << std::endl; // print cmd line options

  preproc::DataType type{ preproc::to_dataType(clo.dataType) };
  switch (type) {

  case preproc::DataType::UnsignedCharacter:
    execute<unsigned char>(clo);
    break;

  case preproc::DataType::UnsignedShort:
    execute<unsigned short>(clo);
    break;

  case preproc::DataType::Float:
    execute<float>(clo);
    break;

  default:
    std::cerr << "Unsupported/unknown datatype: " << clo.dataType << ".\n Exiting...";
    return 1;
    break;
  }

  preproc::logger::shutdown();

  return 0;
}

