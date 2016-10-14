// resample.cpp : Defines the entry point for the console application.
//


#include "grid.h"
#include "cmdline.h"
#include <bd/log/logger.h>

#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <bd/io/datfile.h>

namespace resample
{

template<typename T>
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
  w = ( w / max ) * target;
  h = ( h / max ) * target;
  d = ( d / max ) * target;

  return w * h * d;
}


} // namespace resample

int main(int argc, char const **argv)
{
  resample::CommandLineOptions cmdOpts;
  if (resample::parseThem(argc, argv, cmdOpts) == 0){
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

  bool datFileGiven = ! cmdOpts.datFilePath.empty();
  if (datFileGiven) {
    std::ofstream datFile(cmdOpts.datFilePath, std::ios::binary);
    if (!outFile.is_open()) {
      bd::Err() << "Dat file was not opened: " << cmdOpts.datFilePath << std::endl;
      return 1;
    }
    bd::DatFileData dfd;
    bd::parseDat(cmdOpts.datFilePath, dfd);
    cmdOpts.vol_dims[0] = dfd.rX;
    cmdOpts.vol_dims[1] = dfd.rY;
    cmdOpts.vol_dims[2] = dfd.rZ;
    cmdOpts.dataType = dfd.dataType;
  }

  // read original data into memory
  inFile.seekg(0, std::ios::end);
  std::streampos fileSize{ inFile.tellg() };
  inFile.seekg(0, std::ios::beg);
  char *image{ new char[fileSize] };
  inFile.read(image, fileSize);
  inFile.close();


  size_t orig_c{ cmdOpts.vol_dims[0] }, new_c{ cmdOpts.new_vol_dims[0] };  // col
  size_t orig_r{ cmdOpts.vol_dims[1] }, new_r{ cmdOpts.new_vol_dims[1] };  // row
  size_t orig_s{ cmdOpts.vol_dims[2] }, new_s{ cmdOpts.new_vol_dims[2] };  // slab


  resample::Grid<unsigned char> grid{ orig_c, orig_r, orig_s,
                            reinterpret_cast<unsigned char*>(image) };

  size_t slabSize{ new_c * new_r };
  char *slab{ new char[slabSize] };
  std::memset(slab, 0, slabSize);


  for(size_t s{ 0 }; s < new_s; ++s){
    for(size_t r{ 0 }; r < new_r; r++) {
      for(size_t c{ 0 }; c < new_c; c++) {

        unsigned char ival{
            grid.interpolate(
                { s/float(new_s), r/float(new_r), c/float(new_c) }) };

        slab[c + r*new_c] = ival;

      }
    } // for r

    outFile.write(slab, slabSize);
    if (s%10 == 0) {
      std::cout << "\r Wrote slab: " << s << std::flush;
    }

    memset(slab, 0, slabSize);

  } // for(s

  std::cout << std::endl;

  outFile.flush();
  outFile.close();

  delete [] image;
  delete [] slab;

}

