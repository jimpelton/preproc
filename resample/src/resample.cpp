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


