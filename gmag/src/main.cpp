//
// Created by jim on 3/24/17.
//

#include "cmdline.h"

#include <bd/io/datfile.h>
#include <bd/io/datatypes.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>


template<class Ty>
double
value(Ty const *raw, size_t ix, size_t iy, size_t iz,
      size_t const x, size_t const y, size_t const z)
{
  if (ix < 0) ix = 0;
  else if (ix > x - 1) ix = x - 1;


  if (iy < 0) iy = 0;
  else if (iy > y - 1) iy = y - 1;


  if (iz < 0) iz = 0;
  else if (iz > z - 1) iz = z - 1;


  size_t idx{ ix + x * (iy + iz * y) };
  return raw[idx];
}

template<class Ty>
size_t
start(Ty const * raw, /*std::istream &in,*/ std::ostream &out, size_t const dims[3])
{
  size_t const zmax = dims[2];
  size_t const ymax = dims[1];
  size_t const xmax = dims[0];

  std::cout << "xmax: " << dims[0] << " ymax: " << dims[1] << " zmax: " << dims[2] << std::endl;

  size_t fill = 0;
  size_t totalBytes = 0;

  size_t const buffer_bytes = 8192;
  size_t const max{ buffer_bytes / sizeof(float) };
  float *outvals{ new float[ max ] };

  for (size_t iz{ 0 }; iz < zmax; ++iz)
    for (size_t iy{ 0 }; iy < ymax; ++iy)
      for (size_t ix{ 0 }; ix < xmax; ++ix) {
        double x1, x2, y1, y2, z1, z2;

        x1 = value(raw, ix-1, iy, iz, xmax, ymax, zmax);
        x2 = value(raw, ix+1, iy, iz, xmax, ymax, zmax);

        y1 = value(raw, ix, iy-1, iz, xmax, ymax, zmax);
        y2 = value(raw, ix, iy+1, iz, xmax, ymax, zmax);

        z1 = value(raw, ix, iy, iz-1, xmax, ymax, zmax);
        z2 = value(raw, ix, iy, iz+1, xmax, ymax, zmax);

        double dx = (x1 - x2) * 0.5;
        double dy = (y1 - y2) * 0.5;
        double dz = (z1 - z2) * 0.5;

        float mag = static_cast<float>(dx*dx + dy*dy + dz*dz);
        out.write(reinterpret_cast<const char*>(&mag), sizeof(float));
        fill++;
        if (fill > max) {
          totalBytes += sizeof(float);
          std::cout << "\r " << totalBytes << " bytes written.";
          fill = 0;
        }

//        if (fill < max) {
//          outvals[fill] = mag;
//          fill += 1;
//        } else {
//          char const *p = reinterpret_cast<char const *>(outvals);
//          out.write(p, fill*sizeof(float));
//          totalBytes += fill*sizeof(float);
//          std::cout << "\r " << totalBytes << " bytes written.";
//          fill = 0;
//        }
      } // for

//  if (fill > 0) {
//    char const *p = reinterpret_cast<char const *>(outvals);
//    out.write(p, fill*sizeof(float));
//    totalBytes += fill*sizeof(float);
//    std::cout << "\r " << totalBytes << " bytes written.";
//  }
  out.flush();
  std::cout << std::endl;
  delete [] outvals;
  return totalBytes;
}

template<class Ty>
void
go(size_t const dims[3], std::string const &rawPath, std::string const &outPath)
{
  char *rawData{ nullptr };

  try {
    size_t const bytes{ dims[0] * dims[1] * dims[2] * sizeof(Ty) };
    rawData = new char[ bytes ];
    std::cout << "Allocated " << bytes << " bytes." << std::endl;
  } catch (std::runtime_error &e) {
    std::cerr << "Could not allocate memory: " << e.what() << std::endl;
    rawData = nullptr;
  }

  if (! rawData) return;

  std::ifstream rawfile(rawPath, std::ios::binary);
  if (! rawfile.is_open()) {
    std::cerr << "Could not open raw file: " << rawPath << std::endl;
    return;
  }
  rawfile.read(rawData, dims[0] * dims[1] * dims[2] * sizeof(Ty));
  rawfile.close();

  std::ofstream outfile(outPath, std::ios::binary);
  if (! outfile.is_open()) {
    std::cerr << "Could not open output file: " << outPath << std::endl;
    return;
  }

  start<Ty>(reinterpret_cast<Ty const *>(rawData), outfile, dims);

  outfile.flush();
  outfile.close();
  delete [] rawData;
}

int
main(int argc, char const *argv[])
{
  gmag::CommandLineOptions clo;
  if (gmag::parseThem(argc, argv, clo) == 0) {
    std::cout << "Use -h for command line options." << std::endl;
    return 1;
  }

  std::string path = clo.rawFilePath;
  std::string outPath = clo.outputFilePath;
  bd::DatFileData dat;
  bd::parseDat(clo.datFilePath, dat);
  size_t dims[3] { dat.rX, dat.rY, dat.rZ };

  std::cout << "Datatype: " << bd::to_string(dat.dataType)
            << "\n dims: " << dat.rX << ", " << dat.rY << ", " << dat.rZ
            << std::endl;


  switch(dat.dataType) {
    case bd::DataType::UnsignedCharacter:

      go<uint8_t>(dims, path, outPath);
      break;
    case bd::DataType::UnsignedShort:
      go<uint16_t>(dims, path, outPath);
      break;
    case bd::DataType::Float:
    default:
      go<float>(dims, path, outPath);
      break;
  }

  return 0;
}

