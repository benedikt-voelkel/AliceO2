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

/// \file MCUtils.h
/// \brief Utility functions for MC particles
/// \author S. Wenzel - November 2021

#ifndef O2_MCUTILS_H
#define O2_MCUTILS_H

#include <SimulationDataFormat/MCTrack.h>
#include "TPDGCode.h"

namespace o2
{
namespace mcutils
{
/// A couple of functions to query on MC tracks ( that needs navigation within the global container
/// of available tracks. It is a class so as to make it available for interactive ROOT more easily.
class MCTrackNavigator
{
 public:
  /// Function to determine if a MC track/particle p is a primary according to physics criteria.
  /// Needs the particle as input, as well as the whole navigable container of particles
  /// (of which p needs to be a part itself). The container can be fetched via MCKinematicsReader.
  static bool isPhysicalPrimary(o2::MCTrack const& p, std::vector<o2::MCTrack> const& pcontainer);

  /// return true of particle is to be kept for physics analysis in any case
  /// (follows logic used in particle stack class
  static bool isKeepPhysics(o2::MCTrack const& p, std::vector<o2::MCTrack> const& pcontainer);
  static bool isFromPrimaryDecayChain(o2::MCTrack const& p, std::vector<o2::MCTrack> const& pcontainer);

  // some convenience functions for navigation

  /// Given an MCTrack p; Return the first primary mother particle in the upward parent chain (follow
  /// only first mothers). The first primary mother may have further parent (put by the generator).
  /// Return p itself if p is a primary.
  static o2::MCTrack const& getFirstPrimary(o2::MCTrack const& p, std::vector<o2::MCTrack> const& pcontainer);

  /// Given an MCTrack p; Return it's direct mother or nullptr. (we follow only first mother)
  static o2::MCTrack const* getMother(o2::MCTrack const& p, std::vector<o2::MCTrack> const& pcontainer);

  /// Given an MCTrack p; Return it's direct daughter or nullptr. (we follow only first daughter)
  static o2::MCTrack const* getDaughter(o2::MCTrack const& p, std::vector<o2::MCTrack> const& pcontainer);

  /// Given an MCTrack p; Return it's first direct daughter or nullptr.
  static o2::MCTrack const* getDaughter0(o2::MCTrack const& p, std::vector<o2::MCTrack> const& pcontainer);
  /// Given an MCTrack p; Return it's second direct daughter or nullptr.
  static o2::MCTrack const* getDaughter1(o2::MCTrack const& p, std::vector<o2::MCTrack> const& pcontainer);

  /// Given an MCTrack p; Fill the complete parent chain (ancestorchain) up to the most fundamental particle (follow only
  /// first mothers).
  // static void getParentChain(o2::MCTrack const& p, std::vector<o2::MCTrack> const& pcontainer, std::vector<o2::MCTrack> &ancestorchain);

  /// query if a track is a direct **or** indirect daughter of a parentID
  /// if trackid is same as parentid it returns true
  /// bool isTrackDaughterOf(int /*trackid*/, int /*parentid*/) const;
  /// we can think about offering some visitor like patterns executing a
  /// user hook on nodes

  ClassDefNV(MCTrackNavigator, 1);
};

/// Determine if a particle (identified by pdg) is stable
inline bool isStable(int pdg)
{
  //
  // Decide whether particle (pdg) is stable
  //

  // All ions/nucleons are considered as stable
  // Nuclear code is 10LZZZAAAI
  if (pdg > 1000000000) {
    return true;
  }

  const int kNstable = 18;
  int pdgStable[kNstable] = {
    kGamma,      // Photon
    kElectron,   // Electron
    kMuonPlus,   // Muon
    kPiPlus,     // Pion
    kKPlus,      // Kaon
    kK0Short,    // K0s
    kK0Long,     // K0l
    kProton,     // Proton
    kNeutron,    // Neutron
    kLambda0,    // Lambda_0
    kSigmaMinus, // Sigma Minus
    kSigmaPlus,  // Sigma Plus
    3312,        // Xsi Minus
    3322,        // Xsi
    3334,        // Omega
    kNuE,        // Electron Neutrino
    kNuMu,       // Muon Neutrino
    kNuTau       // Tau Neutrino
  };

  // this is linear search ---> a hash map or binary search should be more appropriate??
  for (int i = 0; i < kNstable; ++i) {
    if (pdg == std::abs(pdgStable[i])) {
      return true;
    }
  }
  return false;
}

int entangleStatusCodes(int hepmc, int gen)
{
  // This is explicitly testing with 2 bytes
  constexpr size_t s = 2; //sizeof(int);

  // This is with the system implementation
  //constexpr size_t s = sizeof(int);


  // The factor 2 comes from multiplying number of bytes by 8 and dividing by 4 to get quarters of the number of bits
  // HepMC gets 1 quarter of bits and 1 additional bit
  constexpr size_t bitsHepmc = 2 * s + 1;
  // Gen gets 3 quarters of bits minus 1 bit
  constexpr size_t bitsGen = 3 * 2 * s - 1;

  // if negative, set our sign bits
  auto negBitHepmc = (static_cast<int>(hepmc < 0)<<(s*8-1));
  auto negBitGen = (static_cast<int>(gen < 0)<<(bitsGen-1));

  // taking really the absolute value makes sure the implementation for negative numbers is used and we don't have to do any bit magic at this point
  auto absHepmc = std::abs(hepmc);
  auto absGen = std::abs(gen);

  // for HepMC we shift by the entire number of bits that might be taken by generator bits
  auto entangled = (absHepmc << (bitsGen));
  // or with generator bits to have them in
  entangled |= absGen;
  // or with negative bits (might be 0 if these were already positive numbers)
  entangled |= negBitHepmc;
  entangled |= negBitGen;
  
  return entangled;
}

void disentangleStatusCode(int entangled, int& hepmc, int& gen)
{
  // This is explicitly testing with 2 bytes
  constexpr size_t s = 2; //sizeof(int);

  // This is with the system implementation
  //constexpr size_t s = sizeof(int);

  
  // The factor 2 comes from multiplying number of bytes by 8 and dividing by 4 to get quarters of the number of bits
  // HepMC gets 1 quarter of bits and 1 additional bit
  constexpr size_t bitsHepmc = 2 * s + 1;
  // Gen gets 3 quarters of bits minus 1 bit
  constexpr size_t bitsGen = 3 * 2 * s - 1;

  // these are our negative bits
  constexpr int negBitHepmc = (1<<(s*8-1));
  constexpr int negBitGen = (1<<(bitsGen-1));

  // the masks
  constexpr int maskGen = (1<<(bitsGen - 1)) - 1;
  constexpr int maskHepmc = ((1<<(bitsHepmc - 1)) - 1) << (bitsGen);

  // generator is easy, the same bits as final number (up to signed bit)
  gen = (maskGen & entangled);
  // check if negative and multiply
  if (static_cast<int>((negBitGen & entangled)) != 0) {
    gen *= -1;
  };

  // Extract HepMC, first get the bits
  hepmc = (maskHepmc & entangled);
  // shift back to original spot
  hepmc = hepmc >> bitsGen;
  // check if negative and multiply
  if (static_cast<int>((negBitHepmc & entangled)) != 0) {
    hepmc *= -1;
  };

} // namespace mcutils
} // namespace o2

#endif //O2_MCUTILS_H
