//
// Created by Jim Pelton on 3/5/16.
//

#ifndef fileblock_h__
#define fileblock_h__

#include <cstdint>
#include <ostream>
#include <sstream>
#include <limits>
//#include <atomic>


namespace preproc
{

///////////////////////////////////////////////////////////////////////////////
///   \brief Describes a block in the IndexFile.
/// 
///   All values in the FileBlock struct are initialized to 0 by the c'tor.
/// 

///   -----------------------------------------
///   For each block:
///   block index       | 8 bytes unsigned
///   block st. offset  | 8 bytes unsigned
///   --
///   block dims X      | 8 bytes unsigned
///   block dims Y      | 8 bytes unsigned
///   block dims Z      | 8 bytes unsigned
///   --
///   block X pos       | 8 bytes float
///   block Y pos       | 8 bytes float
///   block Z pos       | 8 bytes float
///   --
///   max val           | 8 bytes float
///   min val           | 8 bytes float
///   avg val           | 8 bytes float
///
///   isEmpty           | 4 bytes unsigned
///   -----------------------------------------
///   Dat file section (unimplemented)
///   DAT file sz         | 2 bytes unsigned
///   DAT contents        | n bytes ascii with unix newline chars
///////////////////////////////////////////////////////////////////////////////
struct FileBlock
{

  FileBlock()
      : block_index{ 0 }
      , data_offset{ 0 }
      , voxel_dims{ 0 }
      , world_pos{ 0 }
      , min_val{ std::numeric_limits<decltype(min_val)>::max() }
      , max_val{ std::numeric_limits<decltype(max_val)>::min() }
      , avg_val{ 0.0 }
      , total_val{ 0.0 }
      , is_empty{ 0 }
  { }

  std::string 
  to_string() const;

  uint64_t block_index;    ///< The 1D idx of this block (derived from the i,j,k block-grid coordinates).
  uint64_t data_offset;    ///< Offset into the raw file that the block data starts.
  uint64_t voxel_dims[3];  ///< Dimensions of this block in voxels.
  double world_pos[3];      ///< Cordinates within canonical cube.
  double min_val;           ///< The min value found in this block.
  double max_val;           ///< The largest value found in this block.
  double avg_val;           ///< Average value within this block.
  double total_val;
  uint32_t is_empty;        ///< If this block is empty or not.

}; // struct FileBlock

std::ostream& operator<<(std::ostream&, const FileBlock&);

} // namespace bd

#endif //! fileblock_h__
