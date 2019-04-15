// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file Detector.h
/// \brief Definition of the DetectorConstruction class

#ifndef ALICEO2_BASE_DETECTOR_H_
#define ALICEO2_BASE_DETECTOR_H_

#include <map>
#include <vector>
#include <initializer_list>
#include <memory>

#include "FairDetector.h"  // for FairDetector
#include "FairRootManager.h"
#include "DetectorsBase/MaterialManager.h"
#include "Rtypes.h"        // for Float_t, Int_t, Double_t, Detector::Class, etc
#include <cxxabi.h>
#include <typeinfo>
#include <type_traits>
#include <string>
#include <TMessage.h>
#include "CommonUtils/ShmManager.h"
#include "CommonUtils/ShmAllocator.h"
#include <sys/shm.h>
#include <type_traits>
#include <unistd.h>
#include <cassert>

#include <G4Step.hh>
#include <G4TouchableHistory.hh>
#include <TGeoVolume.h>
#include <TGeoManager.h>
#include <FairGeoNode.h>
#include <FairVolume.h>
#include <TString.h>
#include <TRefArray.h>


class FairMQParts;
class FairMQChannel;

namespace o2 {
namespace base {

class DetectorConstruction;


/// Helper function to create a new sensitive detector from a given
/// DetectorConstruction
template <typename T>
T* createSD(DetectorConstruction* dc)
{
  std::vector<TGeoVolume*> tgeoVolumes;
  dc->getSensitiveVolumes(tgeoVolumes);
  if(tgeoVolumes.size() == 0) {
    return nullptr;
  }

  auto volStore = O2VolumeStore::Instance();
  T* sd = new T();
  for(auto& vol : tgeoVolumes) {
    auto g4Vol = volStore.getG4LV(vol);
    if(g4Vol) {
      g4Vol->SetSensitiveDetector(sd);
    }
  }
}


/// This is the basic class for any AliceO2 DetectorConstruction module, whether it is
/// sensitive or not. DetectorConstruction classes depend on this.
class DetectorConstruction : public FairDetector
{

  public:
    DetectorConstruction(const char* name, Bool_t Active);

    /// Default Constructor
    DetectorConstruction();

    /// Default Destructor
    ~DetectorConstruction() override;

    // Module composition
    void Material(Int_t imat, const char *name, Float_t a, Float_t z, Float_t dens, Float_t radl, Float_t absl,
                  Float_t *buf = nullptr, Int_t nwbuf = 0);

    void Mixture(Int_t imat, const char *name, Float_t *a, Float_t *z, Float_t dens, Int_t nlmat,
                 Float_t *wmat);

    void Medium(Int_t numed, const char *name, Int_t nmat, Int_t isvol, Int_t ifield, Float_t fieldm,
                Float_t tmaxfd, Float_t stemax, Float_t deemax, Float_t epsil, Float_t stmin, Float_t *ubuf = nullptr,
                Int_t nbuf = 0);

    /// Custom processes and transport cuts
    void SpecialCuts(Int_t numed, const std::initializer_list<std::pair<ECut, Float_t>>& parIDValMap);
    /// Set cut by name and value
    void SpecialCut(Int_t numed, ECut parID, Float_t val);

    void SpecialProcesses(Int_t numed, const std::initializer_list<std::pair<EProc, int>>& parIDValMap);
    /// Set process by name and value
    void SpecialProcess(Int_t numed, EProc parID, int val);

    /// Define a rotation matrix. angles are in degrees.
    /// \param nmat on output contains the number assigned to the rotation matrix
    /// \param theta1 polar angle for axis I
    /// \param theta2 polar angle for axis II
    /// \param theta3 polar angle for axis III
    /// \param phi1 azimuthal angle for axis I
    /// \param phi2 azimuthal angle for axis II
    /// \param phi3 azimuthal angle for axis III
    void Matrix(Int_t &nmat, Float_t theta1, Float_t phi1, Float_t theta2, Float_t phi2, Float_t theta3,
                Float_t phi3) const;

    static void setDensityFactor(Float_t density)
    {
      mDensityFactor = density;
    }

    static Float_t getDensityFactor()
    {
      return mDensityFactor;
    }

    //virtual bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {return false;}


    /// declare alignable volumes of detector
    virtual void addAlignableVolumes() const;

    /// Sets per wrapper volume parameters
    virtual void defineWrapperVolume(Int_t id, Double_t rmin, Double_t rmax, Double_t zspan);

    /// Books arrays for wrapper volumes
    virtual void setNumberOfWrapperVolumes(Int_t n);

    virtual void defineLayer(Int_t nlay, Double_t phi0, Double_t r, Int_t nladd, Int_t nmod,
                             Double_t lthick = 0., Double_t dthick = 0., UInt_t detType = 0, Int_t buildFlag = 0);

    virtual void defineLayerTurbo(Int_t nlay, Double_t phi0, Double_t r, Int_t nladd, Int_t nmod,
                                  Double_t width, Double_t tilt, Double_t lthick = 0., Double_t dthick = 0.,
                                  UInt_t detType = 0, Int_t buildFlag = 0);

    // returns global material ID given a "local" material ID for this detector
    // returns -1 in case local ID not found
    int getMaterialID(int imat) const {
      auto& mgr = o2::base::MaterialManager::Instance();
      return mgr.getMaterialID(GetName(), imat);
    }

    // returns global medium ID given a "local" medium ID for this detector
    // returns -1 in case local ID not found
    int getMediumID(int imed) const {
      auto& mgr = o2::base::MaterialManager::Instance();
      return mgr.getMediumID(GetName(), imed);
    }

    // fill the medium index mapping into a standard vector
    // the vector gets sized properly and will be overridden
    void getMediumIDMappingAsVector(std::vector<int>& mapping) {
      auto& mgr = o2::base::MaterialManager::Instance();
      mgr.getMediumIDMappingAsVector(GetName(), mapping);
    }

    // return the name augmented by extension
    std::string addNameTo(const char* ext) const
    {
      std::string s(GetName());
      return s + ext;
    }


    // hook which is called automatically to custom initialize the O2 detectors
    // all initialization not able to do in constructors should be done here
    // (typically the case for geometry related stuff, etc)
    virtual void InitializeO2Detector() = 0;

    // the original FairModule/DetectorConstruction virtual Initialize function
    // calls individual customized initializations and makes sure that the mother Initialize
    // is called as well. Marked final for this reason!
    void Initialize() final
    {
      InitializeO2Detector();
      // make sure the basic initialization is also done
      //FairDetector::Initialize();

      // That is basically replaying what is done is FairDetector::Initialize();
      // TODO Check whether getting the volID directly from the TGeoManager is sufficient
      Int_t NoOfEntries=svList->GetEntries();
      Int_t fMCid;
      FairGeoNode* fN;
      TString cutName;
      TString copysign="#";
      for (Int_t i = 0 ; i < NoOfEntries ; i++ )  {
        FairVolume* aVol = static_cast<FairVolume*>(svList->At(i));
        cutName = aVol->GetName();
        Ssiz_t pos = cutName.Index (copysign, 1);
        if(pos>1) { cutName.Resize(pos); }
        if ( aVol->getModId() == GetModId()  ) {
          auto v = gGeoManager->GetVolume(cutName.Data());
          if(v) {
            aVol->setMCid(v->GetNumber());
            fN=aVol->getGeoNode();
            if (fN) { fN->setMCid(v->GetNumber()); }
          }
        }
      }
    }

    // a second initialization method for stuff that should be initialized late
    // (in our case after forking off from the main simulation setup
    // ... for things that should be setup in each simulation worker separately)
    virtual void initializeLate() = 0;

    // The GetCollection interface is made final and deprecated since
    // we no longer support TClonesArrays
    [[deprecated("Use getHits API on concrete detectors!")]]
    TClonesArray* GetCollection(int iColl) const final;

    // static and reusable service function to set tracking parameters in relation to field
    // returns global integration mode (inhomogenety) for the field and the max field value
    // which is required for media creation
    static void initFieldTrackingParams(int &mode, float &maxfield);

    // Construct a new SD from this DetectorConstruction
    SensitiveDetector* CreateNewSensitiveDetector() {return nullptr;}

  protected:

    // Declare a TGeoVolume as sensitive
    void declareSensitiveVolume(TGeoVolume* tgeoVol)
    {
      mSVs.push_back(tgeoVol);
    }

    DetectorConstruction(const DetectorConstruction &origin);
    DetectorConstruction &operator=(const DetectorConstruction &);

  private:
    /// Mapping of the ALICE internal material number to the one
    /// automatically assigned by geant/TGeo.
    /// This is required to easily being able to copy the geometry setup
    /// used in AliRoot
    std::map<int, int> mMapMaterial; //!< material mapping

    /// See comment for mMapMaterial
    std::map<int, int> mMapMedium;   //!< medium mapping

    static Float_t mDensityFactor; //! factor that is multiplied to all material densities (ONLY for
    // systematic studies)

    /// Vector containing TGeoVolumes which have been declared as being sensitive by the user
    /// The corresponding sensitive detector will be construcyed from these
    std::vector<TGeoVolume*> mSVs;

    ClassDefOverride(DetectorConstruction, 1) // Base class for ALICE Modules
};

// an implementation helper template which automatically implements
// common functionality for deriving classes via the CRT pattern
// (example: it implements the updateHitTrackIndices function and avoids
// code duplication, while at the same time avoiding virtual function calls)
template <typename Det>
class DetImpl : public o2::base::DetectorConstruction
{
 public:
  // offer same constructors as base
  using DetectorConstruction::DetectorConstruction;

 public:

  // implementing CloneModule (for G4-MT mode) automatically for each deriving
  // DetectorConstruction class "Det"; calls copy constructor of Det
  FairModule* CloneModule() const final
  {
    return new Det(static_cast<const Det&>(*this));
  }

  ~DetImpl() override
  {
  }

  ClassDefOverride(DetImpl, 0);
};

}
}

#endif
