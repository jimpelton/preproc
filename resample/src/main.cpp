//
// Created by jim on 2/19/17.
//

#include "cmdline.h"
#include "resample.h"
#include "grid.h"

#include <bd/log/logger.h>
#include <bd/io/indexfile.h>
#include <bd/io/datfile.h>

#include <iostream>
#include <fstream>
#include <cstring>
namespace
{

template<class Ty>
char *
allocateEmptyBuffers(char *mem,
                     bd::BlockingQueue<bd::Buffer<Ty> *> &empty,
                     size_t nBuff,
                     size_t lenBuff)
{

  //    size_t const sz_buf{ szMem / nBuff };
  //    size_t const count{ sz_buf / sizeof(Ty) };
  Ty *p{ reinterpret_cast<Ty *>(mem) };

  for (size_t i{ 0 }; i < nBuff; ++i) {
    bd::Buffer<Ty> *buf{ new bd::Buffer<Ty>(p, lenBuff) };
    empty.push(buf);
    p += lenBuff;
  }

  return reinterpret_cast<char *>(p);

} // allocateEmptyBuffers()

} // namespace


template<class Ty>
void
start()
{

}

int main(int argc, char const **argv)
{
  resample::CommandLineOptions cmdOpts;
  if (resample::parseThem(argc, argv, cmdOpts) == 0) {
    std::cerr << "Please use -h for usage." << std::endl;
    return 1;
  }

  //TODO: populate cmdopts if dat file given.

  std::ifstream inFile(cmdOpts.inFilePath, std::ios::binary);
  if (!inFile.is_open()) {
    bd::Err() << "Input file was not found: " << cmdOpts.inFilePath;
    return 1;
  }
  inFile.close();

  std::ofstream outFile(cmdOpts.outFilePath, std::ios::binary);
  if (!outFile.is_open()) {
    bd::Err() << "Output file could not be opened: " << cmdOpts.outFilePath << std::endl;
    return 1;
  }
  outFile.close();

  std::unique_ptr<bd::IndexFile> indexFile{ nullptr };
  /*if (! cmdOpts.indexFilePath.empty()) {

    indexFile = bd::IndexFile::fromBinaryIndexFile(cmdOpts.indexFilePath);

    if (indexFile == nullptr) {
      bd::Err() << "Index file " << cmdOpts.indexFilePath << " could not be opened.";
      return 1;
    }

//    resample::runFromIndexFile();

  } else*/
  if (!cmdOpts.datFilePath.empty()) {

    // check given dat file exists
    std::ifstream datFile(cmdOpts.datFilePath);
    if (!datFile.is_open()) {
      bd::Err() << "Dat file was not opened: " << cmdOpts.datFilePath << std::endl;
      return 1;
    }
    datFile.close();

    bd::DatFileData dfd;
    bd::parseDat(cmdOpts.datFilePath, dfd);
    cmdOpts.vol_dims[0] = dfd.rX;
    cmdOpts.vol_dims[1] = dfd.rY;
    cmdOpts.vol_dims[2] = dfd.rZ;
    cmdOpts.dataType = dfd.dataType;

    indexFile->getVolume().voxelDims(
        { cmdOpts.vol_dims[0], cmdOpts.vol_dims[1], cmdOpts.vol_dims[2] });
    indexFile->getVolume().

    switch(cmdOpts.dataType) {
      case bd::DataType::UnsignedCharacter:
        start<uint8_t>();
        break;
      case bd::DataType::UnsignedShort:
        start<uint16_t>();
        break;
      case bd::DataType::Float:
        start<float>();
        break;
    }


    glm::u64vec3 dest_dims{ cmdOpts.new_vol_dims[0],
                            cmdOpts.new_vol_dims[1],
                            cmdOpts.new_vol_dims[2] };



    char *mem{ new char[] };








//    size_t orig_c{ cmdOpts.vol_dims[0] }, new_c{ cmdOpts.new_vol_dims[0] }; // col
//    size_t orig_r{ cmdOpts.vol_dims[1] }, new_r{ cmdOpts.new_vol_dims[1] }; // row
//    size_t orig_s{ cmdOpts.vol_dims[2] }, new_s{ cmdOpts.new_vol_dims[2] }; // slab

//    resample::Resample::go(dfd.dataType);


    // read original data into memory


  }

  return 0;
}
