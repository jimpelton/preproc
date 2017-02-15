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


  Writer()
      : Writer{ nullptr, nullptr }
  {
  }


  Writer(bd::BlockingQueue<buffer_type *> *full,
         bd::BlockingQueue<buffer_type *> *empty)
      : m_empty{ empty }
      , m_full{ full }
  {
  }


  virtual ~Writer()
  {
  }


public:

  void
  setFull(bd::BlockingQueue<buffer_type *> *full)
  {
    m_full = full;
  }


  void
  setEmpty(bd::BlockingQueue<buffer_type *> *empty)
  {
    m_empty = empty;
  }


  uint64_t
  operator()(std::ostream &os)
  {
    bd::Info() << "Starting writer loop.";
    while (true) {

      buffer_type *buf{ m_full->pop() };

      // Let worker thread exit if the "magical empty buffer" is encountered
      if (!buf->getPtr()) {
        break;
      }

      os.write(reinterpret_cast<char *>(buf->getPtr()),
               buf->getNumElements() * sizeof(Ty));

      buf->setNumElements(0);

      m_empty->push(buf);
    }

    bd::Info() << "Writer loop finished.";

    return 0;
  }


  static void
  start(Writer<Ty> &w, std::ostream &os)
  {
    w.writer_future =
        std::async([&w](std::ofstream &of) -> uint64_t {
                     return w(of);
                   },
                   std::ref(os));
  }


  uint64_t
  join()
  {
    writer_future.wait();
    return writer_future.get();
  }


private:
  queue_type *m_empty;
  queue_type *m_full;

  std::future<uint64_t> writer_future;

};

template<class Ty>
class Reader
{

public:
  using buffer_type = typename bd::Buffer<Ty>;
  using queue_type = typename bd::BlockingQueue<buffer_type *>;


  Reader()
      : Reader{ nullptr, nullptr }
  {
  }


  Reader(bd::BlockingQueue<buffer_type *> *full,
         bd::BlockingQueue<buffer_type *> *empty)
      : m_empty{ empty }
      , m_full{ full }
  {
  }


  virtual ~Reader()
  {
  }


public:

  void
  setFull(bd::BlockingQueue<buffer_type *> *full)
  {
    m_full = full;
  }


  void
  setEmpty(bd::BlockingQueue<buffer_type *> *empty)
  {
    m_empty = empty;
  }


  uint64_t
  operator()(std::istream &is)
  {
    size_t bytes_read{ 0 };
    bd::Info() << "Starting reader loop.";

    while (true) {
      buffer_type *buf{ m_empty->pop() };
      if (!buf->getPtr()) {
        break;
      }

      is.read(reinterpret_cast<char *>(buf->getPtr()),
              buf->getMaxNumElements() * sizeof(Ty));

      std::streamsize amount{ is.gcount() };
      bytes_read += amount;
      buf->setNumElements(amount / sizeof(Ty));

      m_full->push(buf);

      // entire file has been read.
      if (amount < static_cast<long long>(buf->getMaxNumElements())) {
        break;
      }

    }

    bd::Info() << "Reader loop finished.";

    return bytes_read;
  }


  static void
  start(Reader &r, std::istream &is)
  {
    r.reader_future =
        std::async([&r](std::ifstream &of) -> uint64_t {
                     return r(of);
                   },
                   std::ref(is));
  }


  uint64_t
  join()
  {
    reader_future.wait();
    return reader_future.get();
  }


private:
  queue_type *m_empty;
  queue_type *m_full;

  std::future<uint64_t> reader_future;

};


/// Find block min/maxes for blocks that have voxels in \c buf.
/// \param volume
/// \param blocks A list of blocks to
/// \param buf
template<class Ty>
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

    if (b.min_val > pairs[i].min) {
      b.min_val = pairs[i].min;
    }

    if (b.max_val < pairs[i].max) {
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
      double *>;

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
class RFProc
{

public:

  RFProc()
  {
  }


  virtual
  ~RFProc()
  {
  }


  int
  processRawFile(CommandLineOptions const &clo,
                 bd::Volume const &volume,
                 std::vector<bd::FileBlock> &blocks,
                 bool skipRMap);

//  char*
//  allocateEmptyBuffers(char *mem,
//                       bd::BlockingQueue<bd::Buffer<Ty> *> &empty,
//                       size_t nBuff, size_t lenBuff);

private:

  std::ofstream rmapfile;
  std::ifstream rawfile;

  bd::BlockingQueue<bd::Buffer<Ty> *> rawFull;
  bd::BlockingQueue<bd::Buffer<Ty> *> rawEmpty;
  bd::BlockingQueue<bd::Buffer<double> *> rmapFull;
  bd::BlockingQueue<bd::Buffer<double> *> rmapEmpty;

  Reader<Ty> reader;
  Writer<double> writer;

//  std::future<uint64_t> reader_future;
//  std::future<uint64_t> writer_future;

};

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

  //    double *pDouble{ reinterpret_cast<double *>(p) };
  //    bufferSize = clo.bufferSize / 2 / nBuffDouble;
  //    for (int i{ 0 }; i < nBuffDouble; ++i) {
  //      size_t const count{ bufferSize / sizeof(double) };
  //      bd::Buffer<double> *buf{ new bd::Buffer<double>(pDouble, count) };
  //      rmapEmpty.push(buf);
  //      pDouble += count * i;
  //    }
}

} // namespace


/// \brief Create the relevance map in parallel based on the transfer function.
/// \param clo The command line options
/// \param tempOutFilePath Path that the temp RMap scratch file should be written to.
/// \throws std::runtime_error If the raw file could not be opened.
template<class Ty>
int
RFProc<Ty>::processRawFile(CommandLineOptions const &clo,
                           bd::Volume const &volume,
                           std::vector<bd::FileBlock> &blocks,
                           bool skipRMap)
{
  try {
    // The relevance map is stored in pre-allocated vector.
    // It is only as large as a single buffer from the reader
    // and is flushed to disk after each buffer is analyzed.

    rawfile.open(clo.inFile, std::ios::binary);
    if (!rawfile.is_open()) {
      bd::Err() << "Could not open file " + clo.inFile;
      return -1;
    }

    reader.setFull(&rawFull);
    reader.setEmpty(&rawEmpty);
    writer.setFull(&rmapFull);
    writer.setEmpty(&rmapEmpty);
    Reader<Ty>::start(reader, rawfile);
    // number of buffers for reading the raw file off disk.
    // 8 is just an arbitrary number I chose.
    size_t const nBuffTy{ 8 };
    // number of bytes for each of those buffers.
    // clo.bufferSize/2 means we will use half of the total buffer space
    // for the raw file buffers.
    size_t const szBuffTy{ clo.bufferSize / 2 / nBuffTy };
    // The length of each buffer, in elements
    size_t const lenBuffTy{ szBuffTy / sizeof(Ty) };
    size_t const szDouble{ szBuffTy * sizeof(double) / sizeof(Ty) };
    size_t const nBuffDouble{ clo.bufferSize / 2 / szDouble };

    assert(szDouble * nBuffDouble == szBuffTy * nBuffTy);

    char *mem{ new char[clo.bufferSize] };
    mem = allocateEmptyBuffers<Ty>(mem, rawEmpty, nBuffTy, lenBuffTy);
    allocateEmptyBuffers<double>(mem, rmapEmpty, nBuffDouble, lenBuffTy);

    bd::OpacityTransferFunction tr_func{ };
    // If we are doing relevance mapping, then open rmap output file,
    // load the relevance transfer function,
    // reserve space in the relevance map buffer.
    if (!skipRMap) {

      rmapfile.open(clo.rmapFilePath);
      if (!rmapfile.is_open()) {
        bd::Err() << "Could not open rmap output file: " << clo.rmapFilePath;
        return -1;
      }

      // Generate the transfer function
      tr_func.load(clo.tfuncPath);
      if (tr_func.getNumKnots() == 0) {
        bd::Err() << "Transfer function has size 0.";
        return -1;
      }

      Writer<double>::start(writer, rmapfile);

    } // if(! skipRMap)


    // set up the VoxelOpacityFunction
    preproc::VoxelOpacityFunction<Ty>
        rel_func{ tr_func, volume.min(), volume.max() };

    bd::Buffer<Ty> *
        b{ nullptr };

    tbb::task_scheduler_init
        init(clo.numThreads);

    bd::Info() << "Begin working on shit.";
    while (( b = rawFull.pop()) != nullptr) {

      parallelBlockMinMax(volume, blocks, b);

      if (!skipRMap) {
        bd::Buffer<double> *relBuf{ rmapEmpty.pop() };
        createRelMap(b, rel_func, relBuf, rmapfile);
        rmapFull.push(relBuf);
      }

      rawEmpty.push(b);

    } // while
    bd::Info() << "End working on shit.";

    // push the quit buffer into the writer
    bd::Buffer<double> emptyDouble(nullptr, 0);
    rmapFull.push(&emptyDouble);

    reader.join();
    writer.join();
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
    b.avg_val = b.total_val / ( b.voxel_dims[0] * b.voxel_dims[1] * b.voxel_dims[2] );
  }

  return 0;

} // processRawFile()


} // namespace preproc