// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef ALICEO2_HMPID_DETECTOR_H_
#define ALICEO2_HMPID_DETECTOR_H_

#include <vector>
#include "DetectorsBase/Detector.h"
#include "SimulationDataFormat/BaseHits.h"

class TGeoVolume;

namespace o2
{
namespace hmpid
{

// define HMPID hit type
using HitType = o2::BasicXYZEHit<float>;

class Detector : public o2::Base::DetImpl<Detector>
{
 public:
  Detector(Bool_t active = true);
  ~Detector() override = default;

  std::vector<HitType>* getHits(int iColl) const
  {
    if (iColl == 0) {
      return mHits;
    }
    return nullptr;
  }

  bool ProcessHits(FairVolume* v) override;
  void Register() override;
  void Reset() override;
  void Initialize() override;
  void createMaterials();
  void ConstructGeometry() override;
  void defineSensitiveVolumes();
  void EndOfEvent() override { Reset(); }

  // for the geometry sub-parts
  TGeoVolume* createChamber(int number);

 private:
  std::vector<HitType>* mHits = nullptr; ///!< Collection of HMPID hits
  enum EMedia {
    kAir = 1,
    kRoha = 2,
    kSiO2 = 3,
    kC6F14 = 4,
    kCH4 = 5,
    kCsI = 6,
    kAl = 7,
    kCu = 8,
    kW = 9,
    kNeo = 10,
    kAr = 11
  };

  ClassDefOverride(Detector, 1);
};

} // end namespace hmpid
} // end namespace o2

#endif /* ALICEO2_HMPID_DETECTOR_H_ */
