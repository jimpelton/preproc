//
// Created by Jim Pelton on 8/31/16.
//

#ifndef bd_voxelopacityfilter_h__
#define bd_voxelopacityfilter_h__

#include <bd/volume/transferfunction.h>

#include <limits>
#include <vector>

namespace preproc
{

template<typename Ty>
class VoxelOpacityFunction
{

public:
  /// \brief Create a filter based on the given opacity function.
  /// \note Scalars in OpacityKnots should be normalized data values.
  VoxelOpacityFunction(bd::OpacityTransferFunction const &function,
                       double const dataMin,
                       double const dataMax)
      : m_func{ function }
      , m_dataMin{ dataMin }
      , m_diff{ dataMax - dataMin }
  {
  }


  ~VoxelOpacityFunction()
  {
  }


  /// \brief Return the assigned opacity of the voxel based on the given transfer
  /// function.
  /// \param val The voxel value
  /// \return A float that is the opacity.
  double
  operator()(Ty const &val) const
  {
    double val_norm{ ( val - m_dataMin ) / m_diff };
    return m_func.interpolate(val_norm);
  }

private:
  bd::OpacityTransferFunction const m_func;
  double const m_dataMin;
  double const m_diff;


}; // class VoxelOpacityFilter

} // namespace preproc

#endif // ! voxelopacityfilter_h__
