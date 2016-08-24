////////////////////////////////////////////////////////////////////////////////
// Preprocessor
// Generates index files for simple_blocks viewer.
// Index file format
////////////////////////////////////////////////////////////////////////////////

#include "cmdline.h"

#include <bd/util/util.h>
#include <bd/io/indexfile.h>
#include <bd/volume/fileblockcollection.h>
#include <bd/log/logger.h>
#include <bd/io/datfile.h>

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

void
printBlocksToStdOut(bd::IndexFile const &indexFile) {
  std::cout << "{\n";
  for (bd::FileBlock *block : indexFile.blocks()) {
    std::cout << *block << std::endl;
  }
  std::cout << "}\n";
}

void
writeBlocksToFile(bd::IndexFile const &indexFile, CommandLineOptions const &clo) {

  switch (clo.outputFileType) {

    case OutputType::Ascii: {
      std::string outFileName{ makeFileNameString(clo, ".json") };
      indexFile.writeAsciiIndexFile(outFileName);
      break;
    }

    case OutputType::Binary: {
      std::string outFileName{ makeFileNameString(clo, ".bin") };
      indexFile.writeBinaryIndexFile(outFileName);
      break;
    }

  } //switch
}

template<typename Ty>
void
generateIndexFile(const CommandLineOptions &clo)
{

  float minmax[2];
  minmax[0] = clo.tmin;
  minmax[1] = clo.tmax;

  try {
    std::unique_ptr<bd::IndexFile> indexFile{
      bd::IndexFile::fromRawFile(clo.inFile,
                                 clo.bufferSize,
                                 bd::to_dataType(clo.dataType),
                                 clo.vol_dims,
                                 clo.num_blks,
                                 minmax) };

    writeBlocksToFile(*(indexFile.get()), clo);

    if (clo.printBlocks) {
      printBlocksToStdOut(*(indexFile.get()));
    }

  } catch (std::runtime_error e) {
    //std::cerr << e.what() << std::endl;
    Err() << e.what();
  }
}

/// \brief Open a binary index file and print it to stdout or write it to json file.
void
convert(CommandLineOptions & clo)
{

  std::unique_ptr<bd::IndexFile> index{
      bd::IndexFile::fromBinaryIndexFile(clo.inFile)
  };

  if (clo.printBlocks){

    // Print blocks in json format to standard out.
    index->writeAsciiIndexFile(std::cout);

  } else {

    // Otherwise, just write the blocks to a json text file.
    // We can't use makeFileNameString() because we want to use the binary file's name.

    auto startName = clo.inFile.rfind('/') + 1;
    auto endName = startName + (clo.inFile.size() - clo.inFile.rfind('.'));
    std::string name(clo.inFile, startName, endName);
    name += ".json";
    index->writeAsciiIndexFile(clo.outFilePath + '/' + name);

  }
}

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

} // namespace preproc

///////////////////////////////////////////////////////////////////////////////
int
main(int argc, const char *argv[])
{

  using preproc::CommandLineOptions;
  CommandLineOptions clo;
  if (parseThem(argc, argv, clo) == 0) {
    Err() << "Command line parse error, exiting.";
    bd::logger::shutdown();
    return 1;
  }


  switch(clo.actionType) {
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

