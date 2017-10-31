//
// Created by Jim Pelton on 10/3/16.
//


#include "cmdline.h"
#include "voxelopacityfunction.h"
#include "reader.h"
#include "writer.h"
#include "outputer.h"

#include <bd/io/indexfile.h>
#include <bd/log/logger.h>
#include <bd/tbb/parallelfor_voxelrelevance.h>
#include <bd/volume/transferfunction.h>
#include <bd/tbb/parallelreduce_blockminmax.h>
#include <bd/io/bufferpool.h>
#include <bd/io/buffer.h>
#include <bd/datastructure/blockingqueue.h>
#include <bd/volume/block.h>

#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

namespace preproc
{


/// Find block min/maxes for blocks that have voxels in \c buf.
/// \param volume
/// \param blocks A list of blocks to
/// \param rawData
template<class Ty>
void
parallelBlockMinMax(bd::Volume const &volume,
                    std::vector<bd::FileBlock> &blocks,
                    bd::Buffer<Ty> const *rawData)
{
  bd::ParallelReduceBlockMinMax<Ty> minMax{ &volume, rawData };

  tbb::blocked_range<size_t> range{ 0, rawData->getNumElements() };
  tbb::parallel_reduce(range, minMax);
  bd::MinMaxPairDouble const *pairs{ minMax.pairs() };
  for (size_t i{ 0 }; i < blocks.size(); ++i) {
    bd::FileBlock *b = &blocks[i];

    if (b->min_val > pairs[i].min) {
      b->min_val = pairs[i].min;
    }

    if (b->max_val < pairs[i].max) {
      b->max_val = pairs[i].max;
    }

    b->total_val += pairs[i].total;
  }


} // parallelBlockMinMax


/// Process relevance for voxels in rawData buffer. Write results to rmapData buffer.
/// \tparam Ty  type of raw data (rawData buffer)
/// \param rawData the raw voxel values data.
/// \param rmapData empty buffers to fill with relevance values
/// \param relFunc relevance voxel classifier
//template<class Ty>
//void
//createRelMap(bd::Buffer<Ty> const *rawData,
//             bd::Buffer<double> *rmapData,
//             preproc::VoxelOpacityFunction<Ty> &relFunc)
//{
//
//  using Classifier =
//  bd::ParallelForVoxelClassifier<Ty,
//      preproc::VoxelOpacityFunction<Ty>,
//      double *>;
//
//  // The voxel classifier uses the opacity function to write the
//  // opacity to the rmap.
//  double *rmapPtr{ rmapData->getPtr() };
//  Classifier classifier{ rmapPtr, rawData, relFunc };
//
//  // Process this buffer in parallel with the classifier
//  tbb::blocked_range<size_t> range{ 0, rawData->getNumElements() };
//  tbb::parallel_for(range, classifier);
//
//} // createRelMap


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
class RFProc
{

public:

  RFProc()
    : m_mem{ nullptr }
  {
  }


  virtual
  ~RFProc()
  {
    if (m_mem) {
      delete m_mem;
    }
  }


  /// \brief Create the relevance map in parallel based on the transfer function.
  /// \param clo The command line options
  /// \param tempOutFilePath Path that the temp RMap scratch file should be written to.
  /// \throws std::runtime_error If the raw file could not be opened.
  int
  processRawFile(CommandLineOptions const &clo,
                 bd::Volume const &volume,
                 std::vector<bd::FileBlock> &blocks,
                 bool skipRMap);

  void
  loop(bool skipRMap,
       bd::Volume const &volume,
       std::vector<bd::FileBlock> &blocks,
       preproc::VoxelOpacityFunction<Ty> &relFunc);


private:

  bool
  genRMapData(bd::Buffer<Ty> *rawData,
              preproc::VoxelOpacityFunction<Ty> &relFunc,
              bool no_rmap_buffs);


  std::ofstream m_rmapfile;
  std::ifstream m_rawfile;

  bd::BlockingQueue<bd::Buffer<Ty> *> m_rawFull;
  bd::BlockingQueue<bd::Buffer<Ty> *> m_rawEmpty;
  bd::BlockingQueue<bd::Buffer<double> *> m_rmapFull;
  bd::BlockingQueue<bd::Buffer<double> *> m_rmapEmpty;

  Reader<Ty> m_reader;
  Writer<double> m_writer;

  char *m_mem;

};




template<class Ty>
int
RFProc<Ty>::processRawFile(CommandLineOptions const &clo,
                           bd::Volume const &volume,
                           std::vector<bd::FileBlock> &blocks,
                           bool skipRMap)
{
//  preproc::Outputer outputer;
//  outputer.start();

  try {
    // The relevance map is stored in pre-allocated vector.
    // It is only as large as a single buffer from the reader
    // and is flushed to disk after each buffer is analyzed.

    m_rawfile.open(clo.inFile, std::ios::binary);
    if (!m_rawfile.is_open()) {
      bd::Err() << "Could not open file " + clo.inFile;
      return -1;
    }

    m_reader.setFull(&m_rawFull);
    m_reader.setEmpty(&m_rawEmpty);
    m_writer.setFull(&m_rmapFull);
    m_writer.setEmpty(&m_rmapEmpty);

    m_mem = new char[clo.bufferSize];
    {
      size_t const num_rmap{ 16 };
      double ratio{ 0.0 };
      if (sizeof(Ty) == sizeof(double)) {
        ratio = 0.5;
      } else {
        ratio = sizeof(Ty) / double(sizeof(double));
      }


      // total bytes dedicated to raw buffers.
      size_t const sz_total_raw{ size_t(clo.bufferSize * ratio) };
      // total smapce dedicated to rmap buffers.
      size_t const sz_total_rmap{ clo.bufferSize - sz_total_raw };
      // how long is each buffer? It is based off of the number of rmap buffers
      // and the space allocated to rmap buffers.
      size_t const len_buffers{ size_t(( sz_total_rmap / num_rmap ) / sizeof(double)) };
      size_t const sz_raw{ len_buffers * sizeof(Ty) };
      // how many raw buffs can we make of same length.
      size_t num_raw{ sz_total_raw / sz_raw };

      char *mem = allocateEmptyBuffers<Ty>(m_mem, m_rawEmpty, num_raw, len_buffers);
      allocateEmptyBuffers<double>(mem, m_rmapEmpty, num_rmap, len_buffers);

      bd::Info() << "Allocated: " << num_raw << " raw buffers of length " << len_buffers
                 << ", and " << num_rmap << " rmap buffers of length "
                 << len_buffers << ".";

    }

    bd::OpacityTransferFunction tr_func{ };
    // If we are doing relevance mapping, then open rmap output file,
    // load the relevance transfer function,
    // reserve space in the relevance map buffer.
    if (!skipRMap) {

      m_rmapfile.open(clo.rmapFilePath);
      if (!m_rmapfile.is_open()) {
        bd::Err() << "Could not open rmap output file: " << clo.rmapFilePath;
        return -1;
      }

      // Generate the transfer function
      if (tr_func.load(clo.tfuncPath) < 0) {
        bd::Err() << "Error reading transfer function.";
        return -1;
      }
      if (tr_func.getNumKnots() == 0) {
        bd::Err() << "Transfer function has size 0.";
        return -1;
      }

      Writer<double>::start(m_writer, m_rmapfile);
    } // if(! skipRMap)


    Reader<Ty>::start(m_reader, m_rawfile);

    // set up the VoxelOpacityFunction
    preproc::VoxelOpacityFunction<Ty> rel_func{ 
      tr_func, volume.min(), volume.max() };

    loop(skipRMap, volume, blocks, rel_func);

    // push the quit buffer into the writer
    bd::Buffer<double> emptyDouble(nullptr, 0);
    m_rmapFull.push(&emptyDouble);
    bd::Dbg() << "pushed empty magic buffer.";

    m_reader.join();
    m_rawfile.close();

    if (!skipRMap){
      m_writer.join();
      m_rmapfile.close();
    }

  } catch (std::runtime_error &e) {
    bd::Err() << "Exception in " << __func__ << ": " << e.what();
    return -1;
  }

  // compute block averages
  for (size_t i{ 0 }; i < blocks.size(); ++i) {
    bd::FileBlock &b = blocks[i];
    b.avg_val = b.total_val / ( b.voxel_dims[0] * b.voxel_dims[1] * b.voxel_dims[2] );
  }

  bd::Info() << "Finished processing raw file.";

  return 0;

} // processRawFile()


template <class Ty>
void
RFProc<Ty>::loop(bool skipRMap,
                 bd::Volume const &volume,
                 std::vector<bd::FileBlock> &blocks,
                 preproc::VoxelOpacityFunction<Ty> &relFunc)
{

  bd::Info() << "Begin raw file processing, skip_rmap = " << std::boolalpha << skipRMap;

  bd::Buffer<Ty> *rawData{ nullptr };

  while (true) {
    rawData = m_rawFull.pop();

    if (!rawData->getPtr()) {
      bd::Dbg() << "Got null and empty buffer, breaking work loop.";
      break;
    }

    parallelBlockMinMax(volume, blocks, rawData);

    if (!skipRMap) {
      genRMapData(rawData, relFunc, false);
    }

    m_rawEmpty.push(rawData);

  } // while

}


template<class Ty>
bool
RFProc<Ty>::genRMapData(bd::Buffer<Ty> *rawData,
                        preproc::VoxelOpacityFunction<Ty> &relFunc,
                        bool no_rmap_buffs)
{
  bd::Buffer<double> *rmapData{ nullptr };
  rmapData = m_rmapEmpty.pop();
  if (!rmapData->getPtr()) {
    bd::Dbg() << "No rmap data to get. Returning...";
    return false;
  }

//  createRelMap(rawData, rmapData, relFunc);

  // The voxel classifier uses the opacity function to write the
  // opacity to the rmap.
  double *rmapPtr{ rmapData->getPtr() };
  using Relevance = bd::ParallelForVoxelRelevance<Ty, preproc::VoxelOpacityFunction<Ty>, double *>;
  Relevance relevance{ rmapPtr, rawData, relFunc };
  
  // Process this buffer in parallel with the classifier
  tbb::blocked_range<size_t> range{ 0, rawData->getNumElements() };
  tbb::parallel_for(range, relevance);
  rmapData->setNumElements(rawData->getNumElements());
  m_rmapFull.push(rmapData);

  return true;
}



} // namespace preproc
