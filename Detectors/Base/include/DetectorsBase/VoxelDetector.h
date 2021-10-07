// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file Detector.h
/// \brief Definition of the Detector class

#ifndef ALICEO2_BASE_VOXEL_DETECTOR_H_
#define ALICEO2_BASE_VOXEL_DETECTOR_H_

#include <vector>
#include <memory>

#include <VecGeom/base/FlatVoxelHashMap.h>

namespace o2
{
namespace base
{

template <template<typename, bool> typename V = vecgeom::FlatVoxelHashMapCylinder, typename T=bool, bool B=true>
class VoxelDetector
{
  public:
    VoxelDetector() = default;
    VoxelDetector(vecgeom::Vector3D<float> const& min, vecgeom::Vector3D<float> const& length, int nX, int nY, int nZ)
      : mMin{min}, mLength{length}, mDivisions{nX, nY, nZ}
    {
      defineVoxels(min.x(), min.y(), min.z(), length.x(), length.y(), length.z(), nX, nY, nZ);
    }
    ~VoxelDetector() = default;
    VoxelDetector(VoxelDetector const&) = delete;

    void defineVoxels(float minX, float minY, float minZ, float lengthX, float lengthY, float lengthZ, int nX, int nY, int nZ)
    {
      mMin.Set(minX, minY, minZ);
      mLength.Set(lengthX, lengthY, lengthZ);
      mDivisions.Set(nX, nY, nZ);
      mVoxelizedGeometry = std::make_unique<V<T,B>>(mMin, mLength, nX, nY, nZ);
    }

    template <typename H>
    void registerHit(H const& hit)
    {
      auto key = mVoxelizedGeometry->getVoxelKey(vecgeom::Vector3D<float>(hit.GetX(), hit.GetY(), hit.GetZ()));
      assert(key >= 0);
      assert(key < mDivisions.x() * mDivisions.y() * mDivisions.z());
      if (!mVoxelizedGeometry->isOccupied(key)) {
        mVoxelizedGeometry->addPropertyForKey(key, true);
      }
    }

    template <typename H>
    void filterHits(std::vector<H>& hits) const
    {
      if(!mVoxelizedGeometry->size()) {
        return;
      }
      auto unfiltered = hits;
      hits.clear();
      for (auto& h : unfiltered) {
        vecgeom::Vector3D<float> p(h.GetX(), h.GetY(), h.GetZ());
        if (mVoxelizedGeometry->isOccupied(p)) {
          hits.push_back(h);
        }
      }
      return;
    }

    template <typename H>
    bool keepHit(H const& hit) const
    {
      vecgeom::Vector3D<float> p(hit.GetX(), hit.GetY(), hit.GetZ());
      if (mVoxelizedGeometry->isOccupied(p)) {
        return true;
      }
      return false;
    }

    void reset()
    {
      mVoxelizedGeometry->clear();
    }

  private:
    vecgeom::Vector3D<float> mMin;
    vecgeom::Vector3D<float> mLength;
    vecgeom::Vector3D<int> mDivisions;
    std::unique_ptr<V<T,B>> mVoxelizedGeometry;

  //ClassDefNV(V<T,B>, 1);
};

} // namespace base
} // namespace o2

#endif
