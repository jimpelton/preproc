//
// Created by jim on 8/23/16.
//

#include "cmdline.h"

#include <bd/io/datatypes.h>
#include <bd/log/logger.h>
#include <bd/io/buffer.h>
#include <bd/io/bufferedreader.h>
#include <bd/tbb/parallelminmax.h>
#include <bd/tbb/parallelhistogram.h>
#include <bd/io/datfile.h>

#include <iostream>
#include <fstream>
#include <unistd.h>

namespace rawhist
{

namespace
{
  int histcount[1536];
  double histmin[1536];
  double histmax[1536];

} // namespace


template<typename Ty>
void volumeMinMax(std::string const &path, size_t szbuf, Ty* volMin, Ty* volMax)
{
  bd::BufferedReader<Ty> r{ szbuf };
  if (!r.open(path)) {
    bd::Err() << "File " << path << " was not opened.";
    return;
  }

//  bd::Dbg() << "Opened file: " << path;

  r.start();
  Ty max{ 0 };
  Ty min{ 0 };
  bd::Info() << "Begin min/max computation.";
  while (r.hasNext()) {
    bd::Dbg() << "Waiting for full buffer";
    bd::Buffer<Ty> *buf = r.waitNext();
    bd::Dbg() << "Got a full buffer: " << buf->elements();

    tbb::blocked_range<size_t> range(0, buf->elements());
    bd::ParallelMinMax<Ty> mm(buf);
    tbb::parallel_reduce(range, mm);

    if (max < mm.max_value)
      max = mm.max_value;
    if (min > mm.min_value)
      min = mm.min_value;

    r.waitReturn(buf);
  }

  bd::Info() << "Finished min/max computation.";
  *volMin = min;
  *volMax = max;
}


template<typename Ty>
void hist(std::string const &fileName, size_t szbuf, Ty rawmin, Ty rawmax)
{
  bd::BufferedReader<Ty> r(szbuf);
  if (! r.open(fileName)) {
    bd::Err() << "File " << fileName << " was not opened.";
    return;
  }
  bd::Dbg() << "Opened file: " << fileName;
  r.start();

  bd::Info() << "Begin volume histogram computation.";

  while (r.hasNext()) {
    bd::Dbg() << "Waiting for full buffer";
    bd::Buffer<Ty> *buf = r.waitNext();
    bd::Dbg() << "Got a full buffer: " << buf->elements();

    tbb::blocked_range<size_t> range(0, buf->elements());
    bd::ParallelHistogram<Ty> histo(buf, rawmin, rawmax);
    tbb::parallel_reduce(range, histo);

    // Accumulate this buffer's histo
    for(size_t i{ 0 }; i < 1536; ++i) {
      histcount[i] += histo.get(i);
    }

    r.waitReturn(buf);
  }

  bd::Info() << "Finished volume histogram computation.";
}


void
generate(CommandLineOptions &clo)
{
  // Decide what data type we have and call execute to kick off the processing.
  bd::DataType type{ bd::to_dataType(clo.dataType) };
  std::ifstream s;
  switch (type) {

    case bd::DataType::UnsignedCharacter: {
      unsigned char rawmin, rawmax;
      volumeMinMax<unsigned char>(clo.rawFilePath, clo.bufferSize, &rawmin, &rawmax);
      hist<unsigned char>(clo.rawFilePath, clo.bufferSize, rawmin, rawmax);
    }
      break;

    case bd::DataType::UnsignedShort: {
      unsigned short rawmin, rawmax;
      volumeMinMax<unsigned short>(clo.rawFilePath, clo.bufferSize, &rawmin, &rawmax);
      hist<unsigned short>(clo.rawFilePath, clo.bufferSize, rawmin, rawmax);
    }
      break;

    case bd::DataType::Float: {
      float rawmin, rawmax;
      volumeMinMax<float>(clo.rawFilePath, clo.bufferSize, &rawmin, &rawmax);
      hist<float>(clo.rawFilePath, clo.bufferSize, rawmin, rawmax);
    }
      break;

    default:
      bd::Err() << "Unsupported/unknown datatype: " << clo.dataType << ".\n Exiting...";
      break;
  }

  //print histo to stdout.
}

} // namespace rawhist


int main(int argc, const char *argv[])
{
  rawhist::CommandLineOptions clo;
  if (rawhist::parseThem(argc, argv, clo) == 0) {
    std::cout << "No arguments provided.\nPlease use -h for usage info."
              << std::endl;
    return 1;
  }

  if(! clo.datFilePath.empty()) {
    bd::DatFileData dat;
    bd::parseDat(clo.datFilePath, dat);
    clo.dataType = bd::to_string(dat.dataType);
  }

  rawhist::generate(clo);

  return 0;
}


