//
// Created by Jim Pelton on 4/26/16.
//

#ifndef preproc_volume_h__
#define preproc_volume_h__

#include <glm/glm.hpp>
namespace preproc
{

/// \brief Describes a region of uniformly sized blocks
class Region
{
public:
  Region()
      : Region{{ 0, 0, 0 }, {0, 0, 0}}
  { }

  Region(const glm::u64vec3 &blockDims, const glm::u64vec3 &count)
      : m_blockDims { blockDims }
      , m_count{ count }
      , m_voxStart{ 0, 0, 0 }
  { }

  ~Region() { }

  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Get/Set dimensions of the blocks in this region.
  /////////////////////////////////////////////////////////////////////////////////
  void block_dims(const glm::u64vec3 &dims);
  const glm::u64vec3& block_dims() const;
  glm::u64vec3& block_dims();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get/Set the number of blocks along each axis.
  //////////////////////////////////////////////////////////////////////////////
  const glm::u64vec3& block_count() const;
  void block_count(const glm::u64vec3 &);
//  glm::u64vec3& block_count();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get/Set the starting voxel.
  //////////////////////////////////////////////////////////////////////////////
  const glm::u64vec3& vox_start() const;
  void vox_start(const glm::u64vec3& m);

private:
  glm::u64vec3 m_blockDims; ///< Dimensions of a block, in voxels.
  glm::u64vec3 m_count; ///< Number of blocks Region is divided into.
  glm::u64vec3 m_voxStart; ///< col,row,slab of the voxel start of these blocks.

};

class Volume
{

public:
  Volume() : Volume({}, {})
  { }

  Volume(const glm::u64vec3 &volDims, const glm::u64vec3 &numBlocks)
    : m_lowerRegion{ volDims / numBlocks, numBlocks }
    , m_volDims{ volDims }
    , m_volMax{ std::numeric_limits<double>::min() }
    , m_volMin{ std::numeric_limits<double>::max() }
    , m_volAvg{ 0.0 }
  { }

  ~Volume() { }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get/Set the volume's dimensions in voxels
  //////////////////////////////////////////////////////////////////////////////
  const glm::u64vec3 & dims() const;
  void dims(const glm::u64vec3 & voldims);

  double min() const;
  void min(double);
  double max() const;
  void max(double);
  double avg() const;
  void avg(double);

  const Region & lower() const;
  Region& lower();
  void lower(const Region &lower);

//  const Region & upper() const { return m_upperRegion; }
//  Region & upper() { return m_upperRegion; }
//  void upper(const Region &upper) { m_upperRegion = upper; }

private:

  Region m_lowerRegion;

  /// \brief The last row,col,slab of blocks in the volume which contains the
  ///        smaller set of blocks for the remaining voxels that didn't fit into
  ///        the blocks of the lower volume.
  //Region m_upperRegion;

  glm::u64vec3 m_volDims;   ///< Volume dimensions in voxels.
  double m_volMax;          ///< Max value found in volume.
  double m_volMin;          ///< Min value found in volume.
  double m_volAvg;          ///< Avg value found in volume.

};

} // namespace preproc

#endif // ! preproc_volume_h__
