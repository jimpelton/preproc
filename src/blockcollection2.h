#ifndef blockcollection2_h__
#define blockcollection2_h__

#include "fileblock.h"
#include "bufferedreader.h"
#include "logger.h"
#include "util.h"


#include <glm/glm.hpp>

#include <thrust/host_vector.h>
#include <thrust/pair.h>
#include <thrust/extrema.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <iterator>


namespace preproc
{

//////////////////////////////////////////////////////////////////////////////
/// \brief Creates a list of blocks from a large binary raw file. The data type
///        in the binary raw file is supplied by the template parameter.
///
/// \param Ty Data type in the istream this BlockCollection will be
///            generated from.
//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
class BlockCollection2
{
public:


public:

  BlockCollection2();


  BlockCollection2(glm::u64vec3 volDims, glm::u64vec3 numBlocks);


  ~BlockCollection2();


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Marks blocks as empty and uploads GL textures if average is outside of [tmin..tmax].
  /// \param rawFile[in] Volume data set
  /// \param tmin[in] min average block value to filter against.
  /// \param tmax[in] max average block value to filter against.
  ///////////////////////////////////////////////////////////////////////////////
//  template<typename ClassifierFunc>
  void filterBlocks(const std::string &file, size_t bufSize,
//      ClassifierFunc isEmpty,
      float tmin = 0.0f, float tmax = 1.0f,
      bool normalize = false);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Read a single block into buffer \c out.
  /// \param b[in]      The FileBlock for the block that will be read.
  /// \param index[in]  i,j,k coords of the block whos data to get.
  /// \param infile[in] The raw data file.
  /// \param out[out]   Destination space for block data.
  //////////////////////////////////////////////////////////////////////////////
  void fillBlockData(const FileBlock &b, std::istream &infile, Ty *out) const;


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Add a pre-initialized block to this BlockCollection2.
  /// \note  Adds block to non-empty list if block is not empty.
  /// \param b The block to add.
  //////////////////////////////////////////////////////////////////////////////
  void addBlock(const FileBlock &b);


  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Set dimensinos of blocks in voxels
  /////////////////////////////////////////////////////////////////////////////////
  void blockDims(const glm::u64vec3 &dims);


  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Get dimensinos of blocks in voxels
  /////////////////////////////////////////////////////////////////////////////////
  glm::u64vec3 blockDims() const;


  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Get the volume's dimensions in voxels
  /////////////////////////////////////////////////////////////////////////////////
  glm::u64vec3 volDims() const;


  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Set the volume's dimensions in voxels
  /////////////////////////////////////////////////////////////////////////////////
  void volDims(const glm::u64vec3 &voldims);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the number of blocks along each axis.
  //////////////////////////////////////////////////////////////////////////////
  glm::u64vec3 numBlocks() const;


  //////////////////////////////////////////////////////////////////////////////
  double volMin() const
  { return m_volMin; }


  //////////////////////////////////////////////////////////////////////////////
  double volMax() const
  { return m_volMax; }


  //////////////////////////////////////////////////////////////////////////////
  double volAvg() const
  { return m_volAvg; }

  //////////////////////////////////////////////////////////////////////////////
  const std::vector<FileBlock *> &
  blocks() const
  {
    return m_blocks;
  }


  //////////////////////////////////////////////////////////////////////////////
  const std::vector<FileBlock *> &
  nonEmptyBlocks() const
  {
    return m_nonEmptyBlocks;
  }

private:

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Initializes \c nb blocks so that they fit within the extent of \c vd.
  //////////////////////////////////////////////////////////////////////////////
  void initBlocks();


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Compute and save a few stats from provided raw file.
  //////////////////////////////////////////////////////////////////////////////
  void computeVolumeStatistics(BufferedReader<Ty> &r);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Compute and save a few stats for each block.
  //////////////////////////////////////////////////////////////////////////////
  void computeBlockStatistics(BufferedReader<Ty> &r);


  glm::u64vec3 m_blockDims; ///< Dimensions of a block in voxels.
  glm::u64vec3 m_volDims;   ///< Volume dimensions in voxels.
  glm::u64vec3 m_numBlocks; ///< Number of blocks volume is divided into.

  double m_volMax; ///< Max value found in volume.
  double m_volMin; ///< Min value found in volume.
  double m_volAvg; ///< Avg value found in volume.

  std::vector<FileBlock *> m_blocks;
  std::vector<FileBlock *> m_nonEmptyBlocks;


}; // class BlockCollection2


//#include "blockcollection2_impl.hpp"



template<typename Ty>
BlockCollection2<Ty>::BlockCollection2()
    : BlockCollection2({ 0, 0, 0 }, { 0, 0, 0 })
{
}

///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BlockCollection2<Ty>::BlockCollection2
(
    glm::u64vec3 volDims,
    glm::u64vec3 numBlocks
)
  : m_blockDims{ volDims/numBlocks }
  , m_volDims{ volDims }
  , m_numBlocks{ numBlocks }
  , m_volMax{ std::numeric_limits<decltype(m_volMax)>::min() }
  , m_volMin{ std::numeric_limits<decltype(m_volMin)>::max() }
  , m_volAvg{ 0.0 }
  , m_blocks{ }
  , m_nonEmptyBlocks{ }
{
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BlockCollection2<Ty>::~BlockCollection2()
{
  std::cout << "BlockCollection2 destructor\n";
}

///////////////////////////////////////////////////////////////////////////////
// nb: number of blocks
// vd: volume voxel dimensions
// blocks: out parameter to be filled with blocks.
template<typename Ty>
void
BlockCollection2<Ty>::initBlocks()
{
  const glm::u64vec3& nb = m_numBlocks;
  // reset volume dimensions based on number of blocks.
  m_volDims = m_blockDims * m_numBlocks;
  const glm::u64vec3& vd = m_volDims;


  // block world dims
  glm::vec3 wld_dims{ 1.0f/glm::vec3(nb) };

  Info() << "Starting block init: "
      "Number of blocks: " <<
          nb.x << ", " << nb.y << ", " << nb.z <<
      " Volume dimensions: "
          <<  vd.x << ", " << vd.y << ", " << vd.z <<
      " Block dimensions: "
          << ", " << wld_dims.x << ", " << wld_dims.y << ", " << wld_dims.z;


  // Loop through all our blocks (identified by <bx,by,bz>) and populate block fields.
  for (auto bz = 0ull; bz<nb.z; ++bz)
    for (auto by = 0ull; by<nb.y; ++by)
      for (auto bx = 0ull; bx<nb.x; ++bx) {
        // i,j,k block identifier
        const glm::u64vec3 blkId{ bx, by, bz };
        // lower left corner in world coordinates
        const glm::vec3 worldLoc{ wld_dims*glm::vec3(blkId)-0.5f }; // - 0.5f;
        // origin (centroid) in world coordiates
        const glm::vec3 blk_origin{ (worldLoc+(worldLoc+wld_dims))*0.5f };
        // voxel start of block within volume
        const glm::u64vec3 startVoxel{ blkId*m_blockDims };

        FileBlock *blk = new FileBlock(); // { std::make_shared<FileBlock>() };
        blk->block_index = preproc::to1D(bx, by, bz, nb.x, nb.y);
        blk->data_offset = preproc::to1D(startVoxel.x, startVoxel.y, startVoxel.z, m_volDims.x, m_volDims.y)*sizeof(Ty);
        blk->voxel_dims[0] = static_cast<decltype(blk->voxel_dims[0])>(m_blockDims.x);
        blk->voxel_dims[1] = static_cast<decltype(blk->voxel_dims[1])>(m_blockDims.y);
        blk->voxel_dims[2] = static_cast<decltype(blk->voxel_dims[2])>(m_blockDims.z);
        blk->world_pos[0] = blk_origin.x;
        blk->world_pos[1] = blk_origin.y;
        blk->world_pos[2] = blk_origin.z;

        m_blocks.push_back(blk);
      }

  Info() << "Finished block init: total blocks is " << m_blocks.size();
}

//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BlockCollection2<Ty>::computeVolumeStatistics(BufferedReader<Ty> &r)
{
  Info() << "Computing volume statistics...";


//  r.reset();
//  const Ty *ptr = r.buffer_ptr();
//  const thrust::host_vector<Ty> &buf = r.buffer();
//
////  thrust::host_vector<Ty> h_vec;
////  h_vec.reserve(r.bufferSizeElements());
//
  while(r.hasNextFill()) {
//    decltype(r.next()) buf = r.next();

    std::cerr << "Filling buffer.\n";
    //std::cerr << "Read " << elems << " elements\n";

//    auto buf1 = buf.first;
//    auto bufend1 = buf.second;

    std::cerr << "Find min max.\n";
//    auto minmax = thrust::minmax_element(buf.first, buf.second);

    std::cerr << "Reduce.\n";
//    auto sum = thrust::reduce(buf1, bufend1);

//    m_volAvg += sum;
//    m_volMin = std::min<decltype(m_volMin)>(m_volMin, *(minmax.first));
//    m_volMax = std::max<decltype(m_volMax)>(m_volMax, *(minmax.second));


//    for (size_t col{ 0 }; col < elems; ++col) {
//      Ty val{ ptr[col] };
//      m_volMin = std::min<decltype(m_volMin)>(m_volMin, val);
//      m_volMax = std::max<decltype(m_volMax)>(m_volMax, val);
//      m_volAvg  += static_cast<decltype(m_volAvg)>(val);
//    }

  }

  m_volAvg /= m_volDims.x*m_volDims.y*m_volDims.z;

  Info() << "Done computing volume statistics "
        << m_volMin << ", " << m_volMax << ", " << m_volAvg;

}

////////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BlockCollection2<Ty>::computeBlockStatistics(BufferedReader<Ty> &r)
{
  Info() << "Computing block statistics for " << m_blocks.size() << " blocks.";
//  r.reset();

//  const Ty *buf{ r.buffer_ptr() };
//  const thrust::host_vector<Ty> &buf = r.buffer();
//
//  // voxel index within the entire volume
//  size_t vol_idx = 0;
//
//  // the 1D block index of current block
//  size_t blk_idx = 0;
//  FileBlock *currBlock{ m_blocks[blk_idx] };
//
//
////  uint64_t dist_to_right_block_edge = currBlock->voxel_dims[0];
////  uint64_t dist_to_bottom_block_edge = currBlock->voxel_dims[0];
////  uint64_t block_edge = 0 + dist_to_right_block_edge;
//
//  while (r.hasNextFill()) {
//
//    // the number of voxels read last fill.
//    size_t voxels_read = r.fillBuffer();
//
//    if (voxels_read<=0) {
//      std::cout << "\nreadsz: " << voxels_read << " bytes." << std::endl;
//      break;
//    }
//
//    // loop through buffer
//    currBlock = m_blocks[blk_idx];
//    uint64_t voxelsInBlockRow{ currBlock->voxel_dims[0] };
//    size_t vox{ 0 };
//
//    for (; vox < voxelsInBlockRow; ++vox) {
//      Ty val{ buf[vox] };
//
//      currBlock->min_val = std::min<
//          decltype(currBlock->min_val)>(currBlock->min_val, val);
//
//      currBlock->max_val = std::max<
//          decltype(currBlock->max_val)>(currBlock->max_val, val);
//
//      currBlock->avg_val += vox;
//
//    }
//
//    //TODO: compute next block index
//
//    vol_idx += vox;
//
//  } // while(r.hasNext...


    // Determine which block this voxel falls into:         //
    // 1st compute 3D block index from 1D voxel index,      //
    // then compute 1D block index from 3D block index      //
//    auto vX = (vol_idx%m_volDims.x);
//    auto vY = ((vol_idx/m_volDims.x)%m_volDims.y);
//    auto vZ = ((vol_idx/m_volDims.x)/m_volDims.y);
//    size_t blockIdx{
//        preproc::to1D(
//            vX/m_blockDims.x,
//            vY/m_blockDims.y,
//            vZ/m_blockDims.z,
//            m_numBlocks.x,
//            m_numBlocks.y) };

//    for (size_t buf_idx{ 0 }; buf_idx < elems; ++buf_idx, ++vol_idx) {
//      Ty voxel =  buf[buf_idx];
//
//
////      blockIndexFile << xi << "," << yi << ","  << zi << "," << blockIdx << "\n";
//
//      try {
//        // Populate block that this voxel falls into with stats values.
//        FileBlock *b{ m_blocks.at(blockIdx) };
//
//        b->min_val = std::min<decltype(b->min_val)>(b->min_val, voxel);
//        b->max_val = std::max<decltype(b->max_val)>(b->max_val, voxel);
//        b->avg_val += voxel;
//
//      } catch (std::out_of_range e) {
//        std::cerr << e.what() << std::endl;
////        blockIndexFile.flush();
////        blockIndexFile.close();
//        return;
//      }
//    } // for(...
//  } // while(

  for(FileBlock* b : m_blocks) {
    b->avg_val /= b->voxel_dims[0] * b->voxel_dims[1] * b->voxel_dims[2];
  }

  //std::cout << "\nDone computing block statistics.\n";
  Info() << "Done computing block statistics for " << m_blocks.size() << " blocks.";
}


//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BlockCollection2<Ty>::addBlock(const FileBlock& b)
{
  FileBlock* ptr{ new FileBlock(b) }; //{ std::make_shared<FileBlock>(b) };
  m_blocks.push_back(ptr);

  if (!b.is_empty) {
    m_nonEmptyBlocks.push_back(ptr);
  }

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
glm::u64vec3
BlockCollection2<Ty>::blockDims() const
{
  return m_blockDims;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BlockCollection2<Ty>::blockDims(const glm::u64vec3& dims)
{
  m_blockDims = dims;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
glm::u64vec3
BlockCollection2<Ty>::volDims() const
{
  return m_volDims;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BlockCollection2<Ty>::volDims(const glm::u64vec3& voldims)
{
  m_volDims = voldims;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
glm::u64vec3
BlockCollection2<Ty>::numBlocks() const
{
  return m_numBlocks;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
//template<typename ClassifierFunc>
void
BlockCollection2<Ty>::filterBlocks
(
    const std::string &file,
    size_t bufSize,
//    ClassifierFunc isEmpty,
    float tmin,
    float tmax,
    bool normalize
)
{
  initBlocks();

  BufferedReader<Ty> r(bufSize);
  if (! r.open(file)) {
    throw std::runtime_error("Could not open file" + file);
  }
  r.start();

  computeVolumeStatistics(r);
  //computeBlockStatistics(r);

  // total voxels per block
//  size_t numvox{ m_blockDims.x*m_blockDims.y*m_blockDims.z };
//  gl_log("Starting block filtering for %d blocks...", numvox);

//  Ty* image{ new Ty[numvox] };

//  double *normed{ nullptr };
//  if (normalize) {
//    normed = new double[numvox];
//  }

//  for (FileBlock b : m_blocks) {
//    fillBlockData(*b, rawFile, image);

  // Find min/max/avg values
//    b->min_val = std::numeric_limits<decltype(b->min_val)>::max();
//    b->max_val = std::numeric_limits<decltype(b->max_val)>::min();
//    b->avg_val = 0.0;
//    std::for_each(image, image + numvox,
//        [&b](const Ty& t) {
//          b->min_val = std::min<decltype(b->min_val)>(b->min_val, t);
//          b->max_val = std::max<decltype(b->max_val)>(b->max_val, t);
//          b->avg_val += t;
//        });
//    b->avg_val /= numvox;


  // Normalize values in the block, copy to normed
//    if (normalize && normed != nullptr) {
//      const double diff{ m_volMax - m_volMin };
//      std::transform(image, image + numvox, normed,
//         [this, diff](Ty &blkVal) {
//           return (blkVal - this->m_volMin) / diff;
//         });
//    }
//
//
//    // Now, re-find the new min/max/avg values
//    b->min_val = std::numeric_limits<decltype(b->min_val)>::max();
//    b->max_val = std::numeric_limits<decltype(b->max_val)>::min();
//    b->avg_val = 0.0;
//    std::for_each(normed, normed + numvox,
//        [&b](float t) {
//          b->max_val = std::max<decltype(b->max_val)>(b->max_val, t);
//          b->min_val = std::min<decltype(b->min_val)>(b->min_val, t);
//          b->avg_val += t;
//        });
//    b->avg_val /= numvox;
//
//

  for (auto &b : m_blocks){
//    if (isEmpty(b)){
    if (b->avg_val > tmin && b->avg_val < tmax) {
      b->is_empty =  1; //true
    } else {
      b->is_empty = 0; //false
      m_nonEmptyBlocks.push_back(b);
    }
  }
//
//  } // for (FileBlock...
//
//  delete[] image;
//  delete[] normed;

  Info() << m_blocks.size()-m_nonEmptyBlocks.size() << "/" << m_blocks.size() <<
      " blocks marked empty.";
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BlockCollection2<Ty>::fillBlockData
(
    const FileBlock& b,
    std::istream& infile,
    Ty* blockBuffer
) const
{
  // Convert 1D block index to 3D i,j,k indices.
  glm::u64vec3 index{
      b.block_index%m_numBlocks.x,
      (b.block_index/m_numBlocks.x)%m_numBlocks.y,
      (b.block_index/m_numBlocks.x)/m_numBlocks.y
  };

  // start element = block index w/in volume * block size
  const glm::u64vec3 start{ index*m_blockDims };
  // block end element = block voxel start dims + block size
  const glm::u64vec3 end{ start+m_blockDims };

  size_t offset{ b.data_offset };

  // Loop through rows and slabs of volume reading rows of voxels into memory.
  const size_t blockRowLength{ m_blockDims.x };
  for (auto slab = start.z; slab<end.z; ++slab) {
    for (auto row = start.y; row<end.y; ++row) {

      // seek to start of row
      infile.seekg(offset, infile.beg);

      // read the bytes of current row
      infile.read(reinterpret_cast<char*>(blockBuffer), blockRowLength*sizeof(Ty));
      blockBuffer += blockRowLength;

      // offset of next row
      offset = preproc::to1D(start.x, row+1, slab, m_volDims.x, m_volDims.y);
      offset *= sizeof(Ty);
    }
  }
}

} // namespace bd

#endif // !blockcollection2_h__


