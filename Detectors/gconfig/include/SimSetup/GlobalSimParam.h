// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef DETECTORS_GCONFIG_INCLUDE_SIMSETUP_GLOBALSIMPARAM_H_
#define DETECTORS_GCONFIG_INCLUDE_SIMSETUP_GLOBALSIMPARAM_H_

#include "SimConfig/ConfigurableParam.h"
#include "SimConfig/ConfigurableParamHelper.h"

namespace o2
{

struct GlobalSimParam : public o2::conf::ConfigurableParamHelper<GlobalSimParam> {
  int mPAIR = 1;
  int mCOMP = 1;
  int mPHOT = 1;
  int mPFIS = 0;
  int mDRAY = 0;
  int mANNI = 1;
  int mBREM = 1;
  int mHADR = 1;
  int mMUNU = 1;
  int mDCAY = 1;
  int mLOSS = 2;
  int mMULS = 1;
  int mCKOV = 1;

  double mCUTGAM = 1.0E-3; // GeV --> 1 MeV
  double mCUTELE = 1.0E-3; // GeV --> 1 MeV
  double mCUTNEU = 1.0E-3; // GeV --> 1 MeV
  double mCUTHAD = 1.0E-3; // GeV --> 1 MeV
  double mCUTMUO = 1.0E-3; // GeV --> 1 MeV
  double mBCUTE = 1.0E-3;  // GeV --> 1 MeV
  double mBCUTM = 1.0E-3;  // GeV --> 1 MeV
  double mDCUTE = 1.0E-3;  // GeV --> 1 MeV
  double mDCUTM = 1.0E-3;  // GeV --> 1 MeV
  double mPPCUTM = 1.0E-3; // GeV --> 1 MeV
  double mTOFMAX = 1.E10;  // seconds

  // boilerplate stuff + make principal key "HallSim"
  O2ParamDef(GlobalSimParam, "GlobalSim");
};

} // namespace o2

#endif /* DETECTORS_GCONFIG_INCLUDE_SIMSETUP_GLOBALSIMPARAM_H_ */
