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

// -------------------------------------------------------------------------
// -----                    Cave  file                               -----
// -----                Created 26/03/14  by M. Al-Turany              -----
// -------------------------------------------------------------------------
#include "DetectorsBase/MaterialManager.h"
#include "DetectorsBase/Detector.h"
#include "DetectorsPassive/Cave.h"
#include "FairLogger.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
/// cave optimisation
#include "TGeoTube.h"

using namespace o2::Passive;

void Cave::createMaterials()
{
  auto& matmgr = o2::Base::MaterialManager::Instance();
  // Create materials and media
  Int_t isxfld;
  Float_t sxmgmx;
  o2::Base::Detector::initFieldTrackingParams(isxfld, sxmgmx);

  // AIR
  Float_t aAir[4] = { 12.0107, 14.0067, 15.9994, 39.948 };
  Float_t zAir[4] = { 6., 7., 8., 18. };
  Float_t wAir[4] = { 0.000124, 0.755267, 0.231781, 0.012827 };
  Float_t dAir = 1.20479E-3 * 960. / 1014.;

  //
  matmgr.Mixture("CAVE", 2, "Air", aAir, zAir, dAir, 4, wAir);
  //
  matmgr.Medium("CAVE", 2, "Air", 2, 0, isxfld, sxmgmx, 10, -1, -0.1, 0.1, -10);
}

void Cave::ConstructGeometry()
{
  createMaterials();
  auto& matmgr = o2::Base::MaterialManager::Instance();
  /*
  Float_t dALIC[3];

  // TODO: add various options depending on whether ZDC is present or not
  dALIC[0] = 2000;
  dALIC[1] = 2000;
  dALIC[2] = 3000;
  auto cavevol = gGeoManager->MakeBox("cave", gGeoManager->GetMedium("CAVE_Air"), dALIC[0], dALIC[1], dALIC[2]);
  */
  auto cavevol = gGeoManager->MakeTube("cave", gGeoManager->GetMedium("CAVE_Air"), 0., 10., 200.);
  gGeoManager->SetTopVolume(cavevol);
}

void Cave::ModifyGeometry()
{
  /// NB: Cave is TopVOlume (see above!)
  /// Is there a more general way of getting top volume instead of asking by name?
  /// This gives a pointer
  // auto cavevol = gGeoManager->GetVolume("Top volume")
  /// what exactly does this do?
  // cavevol->FindOverlaps();
  /// Build surface volume of all loaded modules except for cave
  /// Loop over modules for doing this. Actually does not need to be done since cavevol is a pointer
  /// can we do gGeoManager->SetTopVolume again here?
  
  /// Pointer to cave node, shape and matrix.
  TGeoNodeMatrix *caveNode = (TGeoNodeMatrix*)gGeoManager->GetTopNode();
  TGeoVolume *caveVolume = caveNode->GetVolume();
  TGeoTube *caveShape = (TGeoTube*)caveVolume->GetShape();
  TGeoTranslation* caveMatrix = (TGeoTranslation*)caveNode->GetMatrix();
  const Double_t *caveOrigin = caveShape->GetOrigin();
  /// All of these are cylinders since we are looking for bounding cylinders of all detector components.
  TObjArray* tubeVols;
  /// New shape of cave with initial (smallish) volume. @todo Better way to infer? Use ITS as the innermost
  /// part or the detecor? In an artificial case, however, the ITS might not be simulated and is hence not 
  /// present at this point...
  /// This tube is enlarged such that the daughter volumes and their shapes are covered.

  LOG(INFO) << "Cave origin, x =  " << caveOrigin[0]
                       << ", y = " << caveOrigin[1]
                       << ", z = " << caveOrigin[2];
  /// Get all daughter nodes
  TIter next(caveNode->GetNodes());
  /// Loop over nodes
  TGeoNode* node = nullptr;
  while( node = (TGeoNode*)next() )
  {
    LOG(INFO) << "CAVE: Optimize for volume " << node->GetVolume()->GetName() << FairLogger::endl;
    /// outer radius for covering tube.
    Double_t params[4];
    /// Get parameters of bounding cylinder. We only need the outer radius for the creation of the 
    /// covering tube.
    node->GetVolume()->GetShape()->GetBoundingCylinder( params );
    /// Outer radius of daughter, apparently bounding cylinder returns the squared radius... 
    Double_t daughterDR = TMath::Sqrt(params[1]);
    Double_t caveDR = caveShape->GetRmax();
    Double_t newDR = caveDR;
    /// dZ extension of current node. Need to call this from TGeoBBox since TGeoShape does not implement this.
    TGeoBBox* daughterBox = (TGeoBBox*)node->GetVolume()->GetShape();
    Double_t daughterDZ = daughterBox->GetDZ();
    Double_t caveDZ = caveShape->GetDZ();
    Double_t newDZ = caveDZ;
    /// In general, the daughter shaopa can have an origin different from O w.r.t. the master frame.
    const Double_t* daughterOriginLocal = daughterBox->GetOrigin();
    
    /// More general transformations of the daughter volume within the mother volume are encoded here.
    TGeoTranslation *daughterMatrix = (TGeoTranslation*)node->GetMatrix();
    /// 
    Double_t daughterOriginMaster[3];
    daughterMatrix->LocalToMaster( daughterOriginLocal, daughterOriginMaster );
    LOG(INFO) << "Origin in Cave, x = " << daughterOriginMaster[0]
                            << ", y = " << daughterOriginMaster[1]
                            << ", z = " << daughterOriginMaster[2];

    /// Is the volume contained the one we already have?
    /// Here, a few checks are necessary and possible rotations/translations encoded in the TGeoMatrix of the 
    /// node might be important. The shape itself does not know about this so we need to combine shape 
    /// information with those provided by the matrix.
    bool modify = false;

    LOG(INFO) << "Cave dZ = " << caveDZ << FairLogger::endl;
    LOG(INFO) << "Cave dR = " << caveDR << FairLogger::endl;
    LOG(INFO) << "Daughter dZ = " << daughterDZ << FairLogger::endl;
    LOG(INFO) << "Daughter dR = " << daughterDR << FairLogger::endl;
    /// At this point assuming that tubes are aligned parrallel to each other, so we only care when the dZ
    /// boundaries of the daughter node exceed the mother volume's ones or when the radius exceeds.
    /// @todo Make this more general.
    Double_t maxDZ;
    if( ( daughterOriginMaster[2] + daughterDZ ) > ( TMath::Abs( daughterOriginMaster[2] - daughterDZ ) ) )
    {
      maxDZ = daughterOriginMaster[2] + daughterDZ;
    }
    else
    {
      maxDZ = TMath::Abs( daughterOriginMaster[2] - daughterDZ );
    }
    /// Now checking the radius...
    /// First, get maximum radius
    Double_t maxDR = TMath::Sqrt( daughterOriginMaster[0]*daughterOriginMaster[0] + 
                                 daughterOriginMaster[1]*daughterOriginMaster[1] ) +
                         daughterDR;


    if( maxDZ > caveDZ )
    {
      newDZ = maxDZ;
      modify = true;
    }
    if( maxDR > caveDR )
    {
      newDR = maxDR;
      modify = true;
    }
    if( modify )
    {
      caveShape->SetTubeDimensions( 0., newDR, newDZ );
      /// @note this is not done implicitly in TGeoTube::SetTubeDimensions. On the other hand, 
      /// TGeoTube::TGeoTube calls this method. So, in order to keep everything clean, we do it here
      /// as well.
      caveShape->ComputeBBox();
    }
  } // End loop over daughters.

}

Cave::Cave() : FairDetector() {}
Cave::~Cave() = default;
Cave::Cave(const char* name, const char* Title) : FairDetector(name, Title, -1) {}
Cave::Cave(const Cave& rhs) : FairDetector(rhs) {}
Cave& Cave::operator=(const Cave& rhs)
{
  // self assignment
  if (this == &rhs)
    return *this;

  // base class assignment
  FairModule::operator=(rhs);
  return *this;
}

FairModule* Cave::CloneModule() const { return new Cave(*this); }
void Cave::FinishPrimary()
{
  LOG(DEBUG) << "CAVE: Primary finished" << FairLogger::endl;
  for (auto& f : mFinishPrimaryHooks) {
    f();
  }
}

bool Cave::ProcessHits(FairVolume*)
{
  LOG(FATAL) << "CAVE ProcessHits called; should never happen" << FairLogger::endl;
  return false;
}

ClassImp(o2::Passive::Cave)
