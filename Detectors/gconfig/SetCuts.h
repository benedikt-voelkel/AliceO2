// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

/** Configuration macro for setting common cuts and processes for G3, G4 and Fluka (M. Al-Turany 27.03.2008)
    specific cuts and processes to g3 or g4 should be set in the g3Config.C, g4Config.C or flConfig.C

*/

void SetCuts()
{
  cout << "SetCuts Macro: Setting Processes.." << endl;

  // ------>>>> IMPORTANT!!!!
  // For a correct comparison between GEANE and MC (pull distributions)
  // or for a simulation without the generation of secondary particles:
  // 1. set LOSS = 2, DRAY = 0, BREM = 1
  // 2. set the following cut values: CUTGAM, CUTELE, CUTNEU, CUTHAD, CUTMUO = 1 MeV or less
  //                                  BCUTE, BCUTM, DCUTE, DCUTM, PPCUTM = 10 TeV
  // (For an explanation of the chosen values, please refer to the GEANT User's Guide
  // or to message #5362 in the PandaRoot Forum >> Monte Carlo Engines >> g3Config.C thread)
  //
  // The default settings refer to a complete simulation which generates and follows also the secondary particles.

  // \note All following settings could also be set in Cave since it is always loaded.
  // Use MaterialManager to set processes and cuts
  auto& mgr = MaterialManager::Instance();
  auto& params = GlobalSimParam::Instance();

  LOG(INFO) << "Set default settings for processes and cuts.";
  mgr.DefaultProcesses({ { EProc::kPAIR, params.mPAIR },    /** pair production */
                         { EProc::kCOMP, params.mCOMP },    /** Compton scattering */
                         { EProc::kPHOT, params.mPHOT },    /** photo electric effect */
                         { EProc::kPFIS, params.mPFIS },    /** photofission */
                         { EProc::kDRAY, params.mDRAY },    /** delta ray */
                         { EProc::kANNI, params.mANNI },    /** annihilation */
                         { EProc::kBREM, params.mBREM },    /** bremsstrahlung */
                         { EProc::kHADR, params.mHADR },    /** hadronic process */
                         { EProc::kMUNU, params.mMUNU },    /** muon nuclear interaction */
                         { EProc::kDCAY, params.mDCAY },    /** decay */
                         { EProc::kLOSS, params.mLOSS },    /** energy loss */
                         { EProc::kMULS, params.mMULS },    /** multiple scattering */
                         { EProc::kCKOV, params.mCKOV } }); /** Cherenkov */

  mgr.DefaultCuts({ { ECut::kCUTGAM, params.mCUTGAM },    /** gammas */
                    { ECut::kCUTELE, params.mCUTELE },    /** electrons */
                    { ECut::kCUTNEU, params.mCUTNEU },    /** neutral hadrons */
                    { ECut::kCUTHAD, params.mCUTHAD },    /** charged hadrons */
                    { ECut::kCUTMUO, params.mCUTMUO },    /** muons */
                    { ECut::kBCUTE, params.mBCUTE },      /** electron bremsstrahlung */
                    { ECut::kBCUTM, params.mBCUTM },      /** muon and hadron bremsstrahlung */
                    { ECut::kDCUTE, params.mDCUTE },      /** delta-rays by electrons */
                    { ECut::kDCUTM, params.mDCUTM },      /** delta-rays by muons */
                    { ECut::kPPCUTM, params.mPPCUTM },    /** direct pair production by muons */
                    { ECut::kTOFMAX, params.mTOFMAX } }); /** time of flight */

  const char* settingProc = mgr.specialProcessesEnabled() ? "enabled" : "disabled";
  const char* settingCut = mgr.specialCutsEnabled() ? "enabled" : "disabled";
  LOG(INFO) << "Special process settings are " << settingProc << ".";
  LOG(INFO) << "Special cut settings are " << settingCut << ".";
  mgr.printProcesses();
  mgr.printCuts();
}
