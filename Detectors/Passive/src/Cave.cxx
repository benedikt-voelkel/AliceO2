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
  Float_t dALIC[3];

  // TODO: add various options depending on whether ZDC is present or not
  dALIC[0] = 2000;
  dALIC[1] = 2000;
  dALIC[2] = 3000;
  auto cavevol = gGeoManager->MakeBox("cave", gGeoManager->GetMedium("CAVE_Air"), dALIC[0], dALIC[1], dALIC[2]);
  gGeoManager->SetTopVolume(cavevol);
}

void Cave::OptimizeGeometry()
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
  auto *caveNode = gGeoManager->GetTopNode();
  auto *caveShape = caveNode->GetVolume()->GetShape();
  auto *caveMatrix = caveNode->GetMatrix();
  auto *caveOrigin[3] = caveShape->GetOrigin();
  /// All of these are cylinders since we are looking for bounding cylinders of all detector components.
  TObjArray* tubes;
  /// New shape of cave
  TGeoPcon *newShape;

  /// Get all daughter nodes
  TIter next(caveNode->GetNodes());
  /// Loop over nodes
  TGeoNode* node = nullptr;
  while( node = (TGeoNode*)next() )
  {
    LOG(INFO) << "CAVE: Optimize for volume " << node->GetVolume()->GetName() << FairLogger::endl;
    /// outer radius for covering tube.
    Double_t params[4];
    node->GetVolume()->GetShape()->GetBoundingCylinder( params );
    /// needed to create covering tube.
    auto dZ = node->GetVolume()->GetShape()->GetDZ();
    auto *daughterOrigin[3] = node->GetVolume()->GetShape()->GetOrigin();
    /// Get TGeoMatrix to adjust relative positions wrt cave origin
    auto *daughterMatrix = node->GetMatrix();
    daughterMatrix->LocalToMaster( 


    auto *tube = new TGeoTube( 0, params[1], dZ );
    tube->SetBowDimensions( tube->GetDX, tube->GetDY, tube->GetDZ, 
    cylinders->Add( new TGeoCylinder( 0, params[1], dZ,  ) );

  }
  


  cavevol->SetShape( newShape )


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
