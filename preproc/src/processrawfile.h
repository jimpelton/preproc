//
// Created by Jim Pelton on 10/3/16.
//


#include "cmdline.h"
#include "voxelopacityfunction.h"

#include <bd/io/indexfile.h>
#include <bd/log/logger.h>
#include <bd/tbb/parallelfor_voxelclassifier.h>
#include <bd/volume/transferfunction.h>
#include <bd/tbb/parallelreduce_blockminmax.h>
#include <bd/io/bufferpool.h>
#include <bd/io/buffer.h>
#include <bd/datastructure/blockingqueue.h>
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <bd/volume/block.h>

namespace preproc
{

template<class Ty>
class Writer
{

public:

  using buffer_type = typename bd::Buffer<Ty>;
  using queue_type = typename bd::BlockingQueue<buffer_type *>;

  Writer(bd::BlockingQueue<buffer_type *> *full,
         bd::BlockingQueue<buffer_type *> *empty)
  : m_empty{ empty }
  , m_full{ full }
  { }

  ~Writer() { }


  uint64_t
  operator()(std::ostream &os)
  {
    while(true) {

      buffer_type *buf{ m_full->pop() };

      // Let worker thread exit if the "magical empty buffer" is encountered
      if (! buf->getPtr()) { break; }

      os.write(reinterpret_cast<char *>(buf->getPtr()),
               buf->getNumElements() * sizeof(Ty));

      buf->setNumElements(0);

      m_empty->push(buf);
    }

    return 0;
  }

private:
  queue_type *m_full;
  queue_type *m_empty;

};

template<class Ty>
class Reader
{

public:
  using buffer_type = typename bd::Buffer<Ty>;
  using queue_type = typename bd::BlockingQueue<buffer_type *>;

  Reader(bd::BlockingQueue<buffer_type *> *full,
         bd::BlockingQueue<buffer_type *> *empty)
  : m_empty{ empty }
  , m_full{ full }
  { }

  uint64_t
  operator()(std::istream &is)
  {
    size_t bytes_read{ 0 };
    while(true) {
      buffer_type *buf{ m_empty->pop() };
      if (! buf->getPtr()) { break; }

      is.read(reinterpret_cast<char *>(buf->getPtr()),
              buf->getNumElements() * sizeof(Ty));

      std::streamsize amount{ is.gcount() };
      bytes_read += amount;
      buf->setNumElements(amount / sizeof(Ty));

      m_full->push(buf);

      if (amount < buf->getMaxNumElements() ) {
        break;
      }

    }

    return bytes_read;
  }

private:
  queue_type *m_full;
  queue_type *m_empty;
};



/// Find block min/maxes for blocks that have voxels in \c buf.
/// \param volume
/// \param blocks A list of blocks to
/// \param buf
template <class Ty>
void
parallelBlockMinMax(bd::Volume const &volume,
                    std::vector<bd::FileBlock> &blocks,
                    bd::Buffer<Ty> const *buf)
{
  bd::ParallelReduceBlockMinMax<Ty> minMax{ &volume, buf };

  tbb::blocked_range<size_t> range{ 0, buf->getNumElements() };
  tbb::parallel_reduce(range, minMax);
  bd::MinMaxPairDouble const *pairs{ minMax.pairs() };
  for (size_t i{ 0 }; i < blocks.size(); ++i) {
    bd::FileBlock &b = blocks[i];

    if (b.min_val > pairs[i].min){
      b.min_val = pairs[i].min;
    }

    if(b.max_val < pairs[i].max){
      b.max_val = pairs[i].max;
    }

    b.total_val += pairs[i].total;
  }



} // parallelBlockMinMax

template<class Ty>
void
createRelMap(bd::Buffer<Ty> const *buf,
             preproc::VoxelOpacityFunction<Ty> &relFunc,
             bd::Buffer<double> *relMap,
             std::ofstream &rmapfile)
{

  using Classifier =
  bd::ParallelForVoxelClassifier<Ty,
                                 preproc::VoxelOpacityFunction<Ty>,
                                 double*>;

  // The voxel classifier uses the opacity function to write the
  // opacity to the rmap.
  double *p{ relMap->getPtr() };
  Classifier classifier{ p, buf, relFunc };

  // Process this buffer in parallel with the classifier
  tbb::blocked_range<size_t> range{ 0, buf->getNumElements() };
  tbb::parallel_for(range, classifier);

  // Write RMap for this buffer out to disk.
//  rmapfile.write(reinterpret_cast<char *>(relMap->getPtr()),
//                 buf->getNumElements() * sizeof(double));

} // createRelMap

template<class Ty>
char*
allocateEmptyBuffers(char *mem, size_t szMem,
  bd::BlockingQueue<bd::Buffer<Ty> *> &empty, size_t nBuff)
{

    size_t const sz_buf{ szMem / nBuff };
    size_t const count{ sz_buf / sizeof(Ty) };
    Ty *p{ reinterpret_cast<Ty *>(mem) };
    for (size_t i{ 0 }; i < nBuff; ++i) {
      bd::Buffer<Ty> *buf{ new bd::Buffer<Ty>(p, count) };
      empty.push(buf);
      p += count * i;
    }

    return reinterpret_cast<char *>(p);

//    double *pDouble{ reinterpret_cast<double *>(p) };
//    bufferSize = clo.bufferSize / 2 / nBuffDouble;
//    for (int i{ 0 }; i < nBuffDouble; ++i) {
//      size_t const count{ bufferSize / sizeof(double) };
//      bd::Buffer<double> *buf{ new bd::Buffer<double>(pDouble, count) };
//      rmapEmpty.push(buf);
//      pDouble += count * i;
//    }
}


/// \brief Create the relevance map in parallel based on the transfer function.
/// \param clo The command line options
/// \param tempOutFilePath Path that the temp RMap scratch file should be written to.
/// \throws std::runtime_error If the raw file could not be opened.
template<class Ty>
int
processRawFile(CommandLineOptions const &clo,
               bd::Volume const &volume,
               std::vector<bd::FileBlock> &blocks,
               bool skipRMap)
{
  try {
    // The relevance map is stored in pre-allocated vector.
    // It is only as large as a single buffer from the reader
    // and is flushed to disk after each buffer is analyzed.

    std::ofstream rmapfile;
    std::ifstream rawfile;
    rawfile.open(clo.inFile, std::ios::binary);
    if (! rawfile.is_open()) {
      bd::Err() << "Could not open file " + clo.inFile;
      return -1;
    }

    bd::BlockingQueue<bd::Buffer<Ty> *> rawFull;
    bd::BlockingQueue<bd::Buffer<Ty> *> rawEmpty;
    bd::BlockingQueue<bd::Buffer<double> *> rmapFull;
    bd::BlockingQueue<bd::Buffer<double> *> rmapEmpty;

    Reader<Ty> reader{ &rawEmpty, &rawFull };
    std::future<uint64_t> reader_future;

    Writer<double> writer{ &rmapEmpty, &rmapFull };
    std::future<uint64_t> writer_future;

    size_t const nBuffTy{ 8 };
    size_t const nBuffDouble{ 4 };
    char *mem{ new char[clo.bufferSize] };
    mem = allocateEmptyBuffers<Ty>(mem, clo.bufferSize / 2, rawEmpty, nBuffTy);
    allocateEmptyBuffers<double>(mem, clo.bufferSize / 2, rmapEmpty, nBuffDouble);

    bd::OpacityTransferFunction trFunc{ };
    // If we are doing relevance mapping, then open rmap output file,
    // load the relevance transfer function,
    // reserve space in the relevance map buffer.
    if (! skipRMap) {

      rmapfile.open(clo.rmapFilePath);
      if (!rmapfile.is_open()) {
        bd::Err() << "Could not open rmap output file: " << clo.rmapFilePath;
        return -1;
      }

      // Generate the transfer function
      trFunc.load(clo.tfuncPath);
      if (trFunc.getNumKnots() == 0) {
        bd::Err() << "Transfer function has size 0.";
        return -1;
      }

      writer_future =
          std::async([&writer](std::ofstream &of) -> uint64_t
                     {
                       return writer(of);
                     },
                     std::ref(rmapfile));

    } // if(! skipRMap)

    reader_future =
        std::async([&reader](std::ifstream &of) -> uint64_t
                   {
                     return reader(of);
                   },
                   std::ref(rawfile));

    // set up the VoxelOpacityFunction
    preproc::VoxelOpacityFunction<Ty>
        relFunc{ trFunc, volume.min(), volume.max() };

    bd::Buffer<Ty> *
        b{ nullptr };

    tbb::task_scheduler_init
        init(clo.numThreads);

    while ((b = rawFull.pop()) != nullptr) {

      parallelBlockMinMax(volume, blocks, b);

      if (! skipRMap) {
        bd::Buffer<double> *relBuf{ rmapFull.pop() };
        createRelMap(b, relFunc, relBuf, rmapfile);
        rmapEmpty.push(relBuf);
      }

      rawEmpty.push(b);

    } // while

    bd::Buffer<double> emptyDouble(nullptr, 0);

    rmapFull.push(&emptyDouble);

    reader_future.wait();
    writer_future.wait();
    rmapfile.close();
    rawfile.close();
  }
  catch (std::runtime_error &e) {
    bd::Err() << "Exception in " << __func__ << ": " << e.what();
    return -1;
  }

  // compute block averages
  for (size_t i{ 0 }; i < blocks.size(); ++i) {
    bd::FileBlock &b = blocks[i];
    b.avg_val = b.total_val / (b.voxel_dims[0] * b.voxel_dims[1] * b.voxel_dims[2]);
  }

  return 0;

} // processRawFile()

} // namespace preproc