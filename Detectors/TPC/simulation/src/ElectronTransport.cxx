// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file ElectronTransport.cxx
/// \brief Implementation of the electron transport
/// \author Andi Mathis, TU München, andreas.mathis@ph.tum.de

#include "TPCSimulation/ElectronTransport.h"

#include <cmath>

using namespace o2::TPC;

ElectronTransport::ElectronTransport() : mRandomGaus(), mRandomFlat()
{
  mRandomGaus.initialize(RandomRing::RandomType::Gaus);
  mRandomFlat.initialize(RandomRing::RandomType::Flat);
}

ElectronTransport::~ElectronTransport() = default;

GlobalPosition3D ElectronTransport::getElectronDrift(GlobalPosition3D posEle, float& driftTime)
{
  const static ParameterGas& gasParam = ParameterGas::defaultInstance();
  const static ParameterDetector& detParam = ParameterDetector::defaultInstance();
  /// For drift lengths shorter than 1 mm, the drift length is set to that value
  float driftl = detParam.getTPClength() - std::abs(posEle.Z());
  if (driftl < 0.01) {
    driftl = 0.01;
  }
  driftl = std::sqrt(driftl);
  const float sigT = driftl * gasParam.getDiffT();
  const float sigL = driftl * gasParam.getDiffL();

  /// The position is smeared by a Gaussian with mean around the actual position and a width according to the diffusion
  /// coefficient times sqrt(drift length)
  GlobalPosition3D posEleDiffusion((mRandomGaus.getNextValue() * sigT) + posEle.X(),
                                   (mRandomGaus.getNextValue() * sigT) + posEle.Y(),
                                   (mRandomGaus.getNextValue() * sigL) + posEle.Z());

  /// If there is a sign change in the z position, the hit has changed sides
  /// This is not possible, but rather just an elongation of the drift time.
  /// In such cases, the old z position of the hit is returned, and the drift time is computed accordingly

  if (posEle.Z() / posEleDiffusion.Z() < 0.f) {
    driftTime = getDriftTime(posEleDiffusion.Z(), -1.f);
    posEleDiffusion.SetZ(posEle.Z());

  } else {
    driftTime = getDriftTime(posEleDiffusion.Z());
  }

  return posEleDiffusion;
}

bool ElectronTransport::isCompletelyOutOfSectorCourseElectronDrift(GlobalPosition3D posEle, const Sector& sector)
{
  const static ParameterGas& gasParam = ParameterGas::defaultInstance();
  const static ParameterDetector& detParam = ParameterDetector::defaultInstance();
  /// For drift lengths shorter than 1 mm, the drift length is set to that value
  float driftl = detParam.getTPClength() - std::abs(posEle.Z());
  if (driftl < 0.01) {
    driftl = 0.01;
  }
  driftl = std::sqrt(driftl);

  /// Three sigma of the expected average transverse diffusion
  const float threeSigmaT = 3.f * driftl * gasParam.getDiffT();

  int secRight = int(sector);
  int secLeft = int(Sector::getLeft(sector));
  const float dSectorBoundaryRight = -mSinsPerSector[secRight] * posEle.X() + mCosinsPerSector[secRight] * posEle.Y();
  const float dSectorBoundaryLeft = -mSinsPerSector[secLeft] * posEle.X() + mCosinsPerSector[secLeft] * posEle.Y();

  if ((dSectorBoundaryLeft > 0 && dSectorBoundaryRight < 0) || (dSectorBoundaryLeft < 0 && dSectorBoundaryRight > 0)) {
    return false;
  }
  if (std::abs(dSectorBoundaryLeft) > threeSigmaT && std::abs(dSectorBoundaryRight) > threeSigmaT) {
    return true;
  }
  return false;
}
