// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/*
 * O2G4MCApplication.h
 *
 *  Created on: Feb 13, 2019
 *      Author: swenzel
 */

#ifndef STEER_INCLUDE_STEER_O2G4MCAPPLICATION_H_
#define STEER_INCLUDE_STEER_O2G4MCAPPLICATION_H_

#include <utility>

#include <FairMCApplication.h>
#include "Rtypes.h" // for Int_t, Bool_t, Double_t, etc
#include "SimConfig/SimCutParams.h"
#include <TGeoManager.h>
#include <THashList.h>


namespace o2
{
namespace steer
{

// O2 specific changes/overrides to FairMCApplication
// Here in particular for custom adjustments to stepping logic
// and tracking limits
class O2G4MCApplication : public FairMCApplication
{
 public:
  O2G4MCApplication() : FairMCApplication(), mCutParams(o2::conf::SimCutParams::Instance()) {}
  O2G4MCApplication(const char* name, const char* title, TObjArray* ModList, const char* MatName) : FairMCApplication(name, title, ModList, MatName), mCutParams(o2::conf::SimCutParams::Instance())
  {
  }

  ~O2G4MCApplication() override = default;

  void BeginEvent() override {;}
  void PreTrack() override {;}
  void BeginPrimary() override {;}
  void Stepping() override {;}
  void FinishPrimary() override {;}
  void PostTrack() override {;}
  void FinishEvent() override {;}

  void ConstructGeometry() override
  {
      // Construct geometry and also fill following member data:
    // - fModVolMap: (volId,moduleId)
    // - fSenVolumes: list of sensitive volumes
    if (!gGeoManager) {
      LOG(fatal) << "gGeoManager not initialized at FairMCApplication::ConstructGeometry\n";
    }

    fModIter->Reset();
    FairModule* Mod=NULL;
    Int_t NoOfVolumes=0;
    Int_t NoOfVolumesBefore=0;
    Int_t ModId=0;

    TObjArray* tgeovolumelist = gGeoManager->GetListOfVolumes();

    while((Mod = dynamic_cast<FairModule*>(fModIter->Next()))) {
      NoOfVolumesBefore=tgeovolumelist->GetEntriesFast();
      Mod->InitParContainers();
      Mod->ConstructGeometry();
      ModId=Mod->GetModId();
      NoOfVolumes=tgeovolumelist->GetEntriesFast();
      for (Int_t n=NoOfVolumesBefore; n < NoOfVolumes; n++) {
        TGeoVolume* v = (TGeoVolume*) tgeovolumelist->At(n);
        fModVolMap.insert(std::pair<Int_t, Int_t >(v->GetNumber(),ModId));
      }
    }

    //  LOG(debug) << "FairMCApplication::ConstructGeometry() : Now closing the geometry";
    int NoOfVolumesBeforeClose = tgeovolumelist->GetEntries();
    gGeoManager->CloseGeometry();   // close geometry
    int NoOfVolumesAfterClose = tgeovolumelist->GetEntries();

    // Check if CloseGeometry has modified the volume list which might happen for
    // runtime shapes (parametrizations,etc). If this is the case, our lookup structures
    // built above are likely out of date. Issue at least a warning here.
    if (NoOfVolumesBeforeClose != NoOfVolumesAfterClose) {
      LOG(error) << "TGeoManager::CloseGeometry() modified the volume list from " << NoOfVolumesBeforeClose
                 << " to " << NoOfVolumesAfterClose << "\n"
                 << "This almost certainly means inconsistent lookup structures used in simulation/stepping.\n";
    }

    Int_t Counter=0;
    TDatabasePDG* pdgDatabase = TDatabasePDG::Instance();
    const THashList *list=pdgDatabase->ParticleList();
    if(list==0)pdgDatabase->ReadPDGTable();
    list =pdgDatabase->ParticleList();
    if(list!=0){
      TIterator *particleIter = list->MakeIterator();
      TParticlePDG *Particle=0;
      while((Particle=dynamic_cast<TParticlePDG*> (particleIter->Next())) && (Counter <= 256)) {
        TString Name= gGeoManager->GetPdgName(Particle->PdgCode());
        //    LOG(info) << Counter <<" : Particle name: "<< Name.Data() << " PDG " << Particle->PdgCode();
        if(Name=="XXX") gGeoManager->SetPdgName(Particle->PdgCode(), Particle->GetName());
        Counter++;
      }
      delete particleIter;
    }
    fModIter->Reset();
    while((Mod = dynamic_cast<FairModule*>(fModIter->Next()))) {
      Mod->ModifyGeometry();
    }

    gGeoManager->RefreshPhysicalNodes(kFALSE);

  }


  void GeneratePrimaries() override
  {
    return;
  }

  // specific implementation of our hard geometry limits
  double TrackingRmax() const override { return mCutParams.maxRTracking; }
  double TrackingZmax() const override { return mCutParams.maxAbsZTracking; }

 protected:
  o2::conf::SimCutParams const& mCutParams; // reference to parameter system
  unsigned long long mStepCounter{ 0 };

  ClassDefOverride(O2G4MCApplication, 1)
};

} // end namespace steer
} // end namespace o2

#endif /* STEER_INCLUDE_STEER_O2G4MCAPPLICATION_H_ */
