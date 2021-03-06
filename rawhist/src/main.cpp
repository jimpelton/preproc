//
// Created by jim on 8/23/16.
//

#include "cmdline.h"

#include <bd/io/datatypes.h>
#include <bd/log/logger.h>
#include <bd/io/buffer.h>
#include <bd/io/bufferedreader.h>
#include <bd/tbb/parallelreduce_minmax.h>
#include <bd/tbb/parallelreduce_histogram.h>
#include <bd/io/datfile.h>

#include <iostream>
#include <fstream>
//#include <unistd.h>
#include <iomanip>
#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>

namespace rawhist
{

namespace
{
int const nBuckets{ 1536 };
int const MAX_IDX{ nBuckets - 1 };
long long histcount[nBuckets];
double histmin[nBuckets];
double histmax[nBuckets];
long long totalcount{ 0 };
double rawmin{ 0 };
double rawmax{ 0 };

} // namespace


template<typename Ty>
void volumeMinMax(std::string const &path, size_t szbuf, double* volMin, double* volMax)
{
  bd::BufferedReader<Ty> r{ szbuf };
  if (!r.open(path)) {
    bd::Err() << "File " << path << " was not opened.";
    return;
  }


  r.start();
  Ty max{ 0 };
  Ty min{ 0 };

  bd::Info() << "Begin min/max computation.";

  bd::Buffer<Ty> *buf{ nullptr };

  while ((buf = r.waitNextFullUntilNone()) != nullptr) {

    tbb::blocked_range<size_t> range(0, buf->getNumElements());
    bd::ParallelReduceMinMax<Ty> mm(buf);

    tbb::parallel_reduce(range, mm);

    if (max < mm.max_value)
      max = mm.max_value;
    if (min > mm.min_value)
      min = mm.min_value;

    r.waitReturnEmpty(buf);

  }

  bd::Info() << "Finished min/max computation.";
  *volMin = min;
  *volMax = max;
}


template<typename Ty>
void hist(std::string const &fileName, size_t szbuf, float rawmin, float rawmax)
{

  bd::BufferedReader<Ty> r(szbuf);
  if (!r.open(fileName)) {
    bd::Err() << "File " << fileName << " was not opened.";
    return;
  }
  r.start();


  bd::Info() << "Begin volume histogram computation.";


  bd::Buffer<Ty> *buf{ nullptr };
  while ((buf = r.waitNextFullUntilNone()) != nullptr) {


//    tbb::blocked_range<size_t> range(0, buf->getNumElements());
//    bd::ParallelReduceHistogram<Ty> histo(buf,
//                                          static_cast<Ty>(rawmin),
//                                          static_cast<Ty>(rawmax) );
//    tbb::parallel_reduce(range, histo);

    Ty const * p{ buf->getPtr() };

    for (size_t i{ 0 }; i < buf->getNumElements(); ++i) {

      Ty val = p[i];

      long long idx{
          static_cast<long long>((val - rawmin)/(rawmax - rawmin) * float(MAX_IDX) + 0.5f) };

      if (idx > MAX_IDX)
        idx = MAX_IDX;

      histcount[idx] += 1;

      if (histmin[idx] > val) {
        histmin[idx] = val;
      }
      if (histmax[idx] < val) {
        histmax[idx] = val;
      }

    }

    totalcount += buf->getNumElements();
    r.waitReturnEmpty(buf);


    // Accumulate this buffer's histo
//    for (size_t i{ 0 }; i < nBuckets; ++i) {
//      histcount[i] += histo.getBuckets()[i];
//
//      if (histo.getHistMin()[i] < histmin[i])
//        histmin[i] = histo.getHistMin()[i];
//
//      if (histo.getHistMax()[i] > histmax[i])
//        histmax[i] = histo.getHistMax()[i];
//    }




  } // while

  bd::Info() << "Finished volume histogram computation.";
}

template<typename Ty>
void doit(CommandLineOptions const &clo)
{
  for (int i = 0; i < nBuckets; ++i) {
    histmin[i] = std::numeric_limits<double>::max();
    histmax[i] = std::numeric_limits<double>::lowest();
    histcount[i] = 0;
  }

  volumeMinMax<Ty>(clo.rawFilePath, clo.bufferSize, &rawmin, &rawmax);
  hist<Ty>(clo.rawFilePath, clo.bufferSize, rawmin, rawmax);
}

void printHisto(std::ostream &os)
{
  os << std::fixed << std::setprecision(6);
  os << "MinMax " << std::setw(20) << rawmin << std::setw(20) << rawmax << '\n';
  os << "#Index Perc Offset Min Max\n";
  double pindex = 0;
  for (int i = 0; i < nBuckets; i++) {
    double pcount = (double)histcount[i] / (double)totalcount;
    if (histcount[i] != 0) {
      os << std::left << std::setw(5) << i << std::left << std::setw(20) << std::setprecision(12) << pcount << std::left << std::setw(20) << std::setprecision(12) << pindex
        << std::left << std::setw(20) << std::setprecision(12) << histmin[i] << std::left << std::setw(20) << std::setprecision(12) << histmax[i] << '\n';
    }
    else {
      os << std::left << std::setw(5) << i << std::left << std::setw(20) << 0.0 << std::left << std::setw(20) << pindex
        << std::left << std::setw(20) << 0.0 << std::left << std::setw(20) << 0.0 << '\n';
    }
    pindex += pcount;
  }

}

void
generate(CommandLineOptions &clo)
{
  // Decide what data type we have and call execute to kick off the processing.
  bd::DataType type{ bd::to_dataType(clo.dataType) };
  switch (type) {

  case bd::DataType::UnsignedCharacter:
    doit<unsigned char>(clo);

    break;

  case bd::DataType::UnsignedShort:
    doit<unsigned short>(clo);

    break;

  case bd::DataType::Float:
    doit<float>(clo);

    break;

  default:
    bd::Err() << "Unsupported/unknown datatype: " << clo.dataType << ".\n Exiting...";
    break;
  }


  if (!clo.outputFilePath.empty()) {

    std::ofstream os(clo.outputFilePath);
    if (!os.is_open()) {
      bd::Err() << "Could not open output file: "
        << clo.outputFilePath << ", using stdout instead.";
      printHisto(std::cout);
    }
    else {
      printHisto(os);
    }
  }
  else {
    printHisto(std::cout);
  }

  //print histo to stdout.

}

} // namespace rawhist


int main(int argc, const char *argv[])
{
  rawhist::CommandLineOptions clo;

  if (rawhist::parseThem(argc, argv, clo) == 0) {

    std::cerr << "No arguments provided.\nPlease use -h for usage info."
      << std::endl;
    return 1;

  }


  if (!clo.datFilePath.empty()) {

    bd::DatFileData dat;
    bd::parseDat(clo.datFilePath, dat);
    clo.dataType = bd::to_string(dat.dataType);

  }


  rawhist::generate(clo);

  return 0;
}


