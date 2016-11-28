// resample.cpp : Defines the entry point for the console application.
//


#include "grid.h"
#include "cmdline.h"

#include <bd/log/logger.h>
#include <bd/volume/block.h>

#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <bd/io/datfile.h>
#include <bd/io/indexfile.h>
#include <bd/io/bufferedreader.h>


namespace resample
{
template <typename T>
T
maxOf(T a, T b, T c)
{
  T max = a;

  if (a < b) {
    max = b;
  }

  if (b < c) {
    max = c;
  }

  return max;
}


size_t
computeSlabNumElements(size_t w, size_t h, size_t d, size_t target)
{
  size_t max{ maxOf<size_t>(w, h, d) };
  w = (w / max) * target;
  h = (h / max) * target;
  d = (d / max) * target;

  return w * h * d;
}


/// \brief Read the data for Block \c b into \c blockBuffer.
/// \param ve - volume voxel extent in x, y dimensions
//template <class Ty>
//void
//fillBlockData(bd::Block const &b,
//              std::istream &infile,
//
//              glm::u64vec2 ve,
//              Ty *blockBuffer)
//{
//  // block's dimensions in voxels
//  glm::u64vec3 const be{ b.voxel_extent() };
//
//  // start element = block index w/in volume * block size
//  const glm::u64vec3 start{ b.ijk() * be };
//
//  // block end element = block voxel start voxelDims + block size
//  const glm::u64vec3 end{ start + be };
//
//  // byte offset into file to read from
//  size_t offset{ b.fileBlock().data_offset };
//
//  // Loop through rows and slabs of volume reading rows of voxels into memory.
//  const size_t blockRowLength{ be.x };
//  for (auto slab = start.z; slab < end.z; ++slab) {
//    for (auto row = start.y; row < end.y; ++row) {
//
//      // seek to start of row
//      infile.seekg(offset, infile.beg);
//
//      // read the bytes of current row
//      infile.read(reinterpret_cast<char *>(blockBuffer), blockRowLength * sizeof(Ty));
//      blockBuffer += blockRowLength;
//
//      // offset of next row
//      offset = bd::to1D(start.x, row + 1, slab, ve.x, ve.y);
//      offset *= sizeof(Ty);
//    }
//  }
//}

template<class T>
bool
go(std::string const &rawFile, bd::IndexFile const &indexFile)
{
  bd::BufferedReader<T> r{ };
  if (! r.open(rawFile)) {
    bd::Err() << "Could not open raw file: " << rawFile;
    return false;
  }

  bd::Buffer<T> *buf{ nullptr };

  while((buf = r.waitNextFullUntilNone()) != nullptr) {
    
  }


}


bool 
runFromIndexFile(std::string const &rawFile, bd::IndexFile const &indexFile)
{
  bd::DataType type{ bd::IndexFileHeader::getType(indexFile.getHeader()) };
  switch (type) {
  case bd::DataType::Character:
    go<int8_t>(rawFile, indexFile);
    break;
  case bd::DataType::UnsignedCharacter:
    go<uint8_t>(rawFile, indexFile);
    break;
  case bd::DataType::Short:
    go<int16_t>(rawFile, indexFile);
    break;
  case bd::DataType::UnsignedShort:
    go<uint16_t>(rawFile, indexFile);
    break;
  case bd::DataType::Float:
    go<float>(rawFile, indexFile);
    break;
  default:
    bd::Err() << "Unsupported data type: " << bd::to_string(type);
    return false;
  }

  return true;
  
}

} // namespace resample

int main(int argc, char const **argv)
{
  resample::CommandLineOptions cmdOpts;
  if (resample::parseThem(argc, argv, cmdOpts) == 0) {
    std::cerr << "Please use -h for usage." << std::endl;
    return 1;
  }

  //TODO: populate cmdopts if dat file given.

  std::ifstream inFile(cmdOpts.inFilePath, std::ios::binary);
  if (! inFile.is_open()) {
    bd::Err() << "Input file was not opened: " << cmdOpts.inFilePath;
    return 1;
  }


  std::ofstream outFile(cmdOpts.outFilePath, std::ios::binary);
  if (! outFile.is_open()) {
    bd::Err() << "Output file was not opened: " << cmdOpts.outFilePath << std::endl;
    return 1;
  }

  std::unique_ptr<bd::IndexFile> indexFile{ nullptr };
  if (! cmdOpts.indexFilePath.empty()) {

    indexFile =
      bd::IndexFile::IndexFile::fromBinaryIndexFile(cmdOpts.indexFilePath);

    if (indexFile == nullptr) {
      bd::Err() << "Index file " << cmdOpts.indexFilePath << " could not be opened.";
      return 1;
    }

    resample::runFromIndexFile(*(indexFile.get()));

  }
  else if (!cmdOpts.datFilePath.empty()) {

 // check given dat file exists
    std::ifstream datFile(cmdOpts.datFilePath, std::ios::binary);
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



    size_t orig_c{ cmdOpts.vol_dims[0] }, new_c{ cmdOpts.new_vol_dims[0] }; // col
    size_t orig_r{ cmdOpts.vol_dims[1] }, new_r{ cmdOpts.new_vol_dims[1] }; // row
    size_t orig_s{ cmdOpts.vol_dims[2] }, new_s{ cmdOpts.new_vol_dims[2] }; // slab

    // read original data into memory
    inFile.seekg(0, std::ios::end);
    std::streampos fileSize{ inFile.tellg() };
    inFile.seekg(0, std::ios::beg);
    char *image{ new char[fileSize] };
    inFile.read(image, fileSize);
    inFile.close();


    resample::Grid<unsigned char> grid{ orig_c, orig_r, orig_s,
      reinterpret_cast<unsigned char*>(image) };

    size_t slabSize{ new_c * new_r };
    char *slab{ new char[slabSize] };
    std::memset(slab, 0, slabSize);


    for (size_t s{ 0 }; s < new_s; ++s) {
      for (size_t r{ 0 }; r < new_r; r++) {
        for (size_t c{ 0 }; c < new_c; c++) {

          unsigned char ival{
            grid.interpolate(
                             { s / float(new_s), r / float(new_r), c / float(new_c) }) };

          slab[c + r * new_c] = ival;

        }
      } // for r

      outFile.write(slab, slabSize);
      if (s % 10 == 0) {
        std::cout << "\r Wrote slab: " << s << std::flush;
      }

      memset(slab, 0, slabSize);

    } // for(s

    std::cout << std::endl;

    outFile.flush();
    outFile.close();

    delete[] image;
    delete[] slab;
  }

  return 0;
}
