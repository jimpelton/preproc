//
// Created by Jim Pelton on 2/28/16.
// Note: this code is based on the code found at scratchapixel. 
// Link:
//   http://www.scratchapixel.com/old/lessons/3d-advanced-lessons/interpolation/trilinear-interpolation/
//
// It has been changed a little bit to work for my situation.

#ifndef resample_grid_h
#define resample_grid_h

#include <cstddef>

namespace resample
{
struct vec3f
{
  float x, y, z;
};

template<typename T>
class Grid
{
public:
  size_t nx, ny, nz; // number of vertices
  size_t nvoxels; // number of voxels (cube)
  const T *data;


  ///
  /// \param nvx Number of verts along x
  /// \param nvy Number of verts along y
  /// \param nvz Number of verts along z
  /// \param data The data to resample.
  Grid(size_t nvx, size_t nvy, size_t nvz, const T *data)
      : nx(nvx)
      , ny(nvy)
      , nz(nvz)
      , nvoxels(nvx * nvy * nvz)
      , data(data)
  {
  }


  ~Grid()
  {
  }


  /// Convert to 1d index
  size_t
  IX(size_t x, size_t y, size_t z)
  {
    if ( x > nx ) {
      x -= 1;
    }

    if ( y > ny ) {
      y -= 1;
    }

    if ( z > nz ) {
      z -= 1;
    }

    return x + nx * ( y + ny * z );
    //return x * nx * ny + y * nx + z;
  }


  T
  bilinear(const T &tx, const T &ty,
           const T &c00, const T &c10,
           const T &c01, const T &c11)
  {
    T a = c00 * ( T(1) - tx ) + c10 * tx;
    T b = c01 * ( T(1) - tx ) + c11 * tx;
    return a * ( T(1) - ty ) + b * ty;
  }


  T
  interpolate(const vec3f &location)
  {
    float gx, gy, gz;
    size_t gxi, gyi, gzi;

    T tx, ty, tz;

    // remap point coordinates to grid coordinates
    gx = location.x * nx;
    gxi = static_cast<size_t>(gx);
    tx = gx - gxi;

    gy = location.y * ny;
    gyi = static_cast<size_t>(gy);
    ty = gy - gyi;

    gz = location.z * nz;
    gzi = static_cast<size_t>(gz);
    tz = gz - gzi;

    const T c000 = data[IX(gxi, gyi, gzi)];
    const T c100 = data[IX(gxi + 1, gyi, gzi)];
    const T c010 = data[IX(gxi, gyi + 1, gzi)];
    const T c110 = data[IX(gxi + 1, gyi + 1, gzi)];
    const T c001 = data[IX(gxi, gyi, gzi + 1)];
    const T c101 = data[IX(gxi + 1, gyi, gzi + 1)];
    const T c011 = data[IX(gxi, gyi + 1, gzi + 1)];
    const T c111 = data[IX(gxi + 1, gyi + 1, gzi + 1)];

    T e = bilinear(tx, ty, c000, c100, c010, c110);
    T f = bilinear(tx, ty, c001, c101, c011, c111);
    return e * ( 1 - tz ) + f * tz;
  }
};

}

#endif // ! resample_grid_h
