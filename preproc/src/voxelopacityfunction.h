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
  VoxelOpacityFunction(const std::vector<bd::OpacityKnot> &function,
                       const Ty &dataMin,
                       const Ty &dataMax)
      : m_func{ function }
      , m_dataMin{ dataMin }
      , m_diff{ static_cast<double>(dataMax - dataMin) }
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
//    double a{ alpha(val_norm) };
    return alpha(val_norm);
  }


  // find the alpha value associated with scalar value v.
  // v assumed between 0..1.
  double
  alpha(double v) const
  {
    bd::OpacityKnot b{ 0, 0 };
    bd::OpacityKnot a{ m_func[0] };
    for (size_t i = 1; i < m_func.size(); ++i) {
      b = m_func[i];

      //TODO: fix unsafe comparison 
      if (v == b.scalar) {
        return b.alpha;
      } else if (v < b.scalar) {
        // v is between a.scalar and b.scalar, so lerp the alpha value.
        return a.alpha * ( 1.0 - v ) + b.alpha * v;
      }

      a = m_func[i];
    }

    return 0;

  }


private:
  const std::vector<bd::OpacityKnot> m_func;
  const double m_dataMin;
  const double m_diff;


}; // class VoxelOpacityFilter

} // namespace bd

#endif // ! voxelopacityfilter_h__
