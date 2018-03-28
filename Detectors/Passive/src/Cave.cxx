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
  
  Float_t dALIC[3];

  // TODO: add various options depending on whether ZDC is present or not
  dALIC[0] = 2000;
  dALIC[1] = 2000;
  dALIC[2] = 3000;
  //auto cavevol = gGeoManager->MakeBox("cave", gGeoManager->GetMedium("CAVE_Air"), dALIC[0], dALIC[1], dALIC[2]);
  
  /// New shape of cave with initial (smallish) volume. @todo Better way to infer? Use ITS as the innermost
  /// part or the detecor? In an artificial case, however, the ITS is not present at this point...
  TGeoVolume* cavevol;
  if(mOptimize)
  {
    cavevol = gGeoManager->MakeTube("cave", gGeoManager->GetMedium("CAVE_Air"), 0., 1., 1.);
    //cavevol = gGeoManager->MakeBox("cave", gGeoManager->GetMedium("CAVE_Air"), 1., 1., 1.);
  }
  else
  {
    cavevol = gGeoManager->MakeBox("cave", gGeoManager->GetMedium("CAVE_Air"), dALIC[0], dALIC[1], dALIC[2]);
  }

  gGeoManager->SetTopVolume(cavevol);
}

/*
 * This method looks for the smallest tube covering all subdetectors. So far it is assumed that the 
 * tubes are always aligned parallel to each other with arbitrary translations w.r.t. each other. A 
 * more general covering shape may be a polycon. Also, rotations of the detector parts need to be 
 * taken into account to generalise this code.
 */
void Cave::ModifyGeometry()
{
  /// Only do modification if requested
  if(mOptimize)
  {
    /// Pointer to cave node, shape and matrix.
    TGeoNodeMatrix *caveNode = static_cast<TGeoNodeMatrix*>(gGeoManager->GetTopNode());
    TGeoTube *caveShape = static_cast<TGeoTube*>(caveNode->GetVolume()->GetShape());
    
    Double_t maxDR = caveShape->GetRmax();
    Double_t minZ = -caveShape->GetDZ();
    Double_t maxZ = -minZ;
    int nDaughters = 0;
    int sumOriginZ = 0;
    /// Get all daughter nodes and loop...
    TIter next(caveNode->GetNodes());
    TGeoNode* node = nullptr;
    while( node = (TGeoNode*)next() )
    {
      LOG(INFO) << "CAVE: Optimize for volume " << node->GetVolume()->GetName() << FairLogger::endl;
                            
      /// We can be sure that all other shapes are derived from this since this is the way it's done.
      TGeoBBox* daughterBox = static_cast<TGeoBBox*>(node->GetVolume()->GetShape());
      /// Compute bounding box which contains a tube and hence gives extrema of tube geometry. Get dimensions.
      daughterBox->ComputeBBox();
      Double_t daughterDX = daughterBox->GetDX();
      Double_t daughterDY = daughterBox->GetDY();
      Double_t daughterDZ = daughterBox->GetDZ();

      /// In general, the daughter shape can have an origin different from O w.r.t. its own frame.
      const Double_t* daughterOriginLocal = daughterBox->GetOrigin();
      /// translation and rotation matrices of the daughter.
      TGeoTranslation *daughterTranslation = static_cast<TGeoTranslation*>(node->GetMatrix());
      TGeoRotation *daughterRotation = static_cast<TGeoRotation*>(node->GetMatrix());
      /// Prepare for actual origin in master/cave volume.
      Double_t daughterOriginMasterTmp[3];
      Double_t daughterOriginMaster[3];
      /// Do transformation.
      daughterTranslation->LocalToMaster( daughterOriginLocal, daughterOriginMasterTmp );
      daughterRotation->LocalToMaster( daughterOriginMasterTmp, daughterOriginMaster );
      /// Add origin Z value and increment number of daughter. Later the average is derived from that.
      sumOriginZ += daughterOriginMaster[2];
      nDaughters++;
      /// Find daughter edges.
      for( int i = 1; i < 3; i++ )
      {
        for( int j = 1; j < 3; j++ )
        {
          for( int k = 1; k < 3; k++ )
          {
            const Double_t x = daughterOriginMaster[0] + TMath::Power( -1, i )*daughterDX;
            const Double_t y = daughterOriginMaster[1] + TMath::Power( -1, j )*daughterDY;
            const Double_t z = daughterOriginMaster[2] + TMath::Power( -1, k )*daughterDZ;
            /// Maximise/minimise Z
            if( z < minZ )
            {
              minZ = z;
            }
            if( z > maxZ )
            {
              maxZ = z;
            }
            /// maxR is derived from bounding box edges so the resulting cylinder may have a larger
            /// radius than the daughter cylinder.
            if( TMath::Sqrt(x*x + y*y) > maxDR )
            {
              maxDR = TMath::Sqrt(x*x+y*y);
            }
          }
        }
      } // End loop over edges.
      
      
    } // End loop over daughters.
    
    /// derive the minimum Z extension needed for the cave.
    Double_t newCaveDZ = (maxZ - minZ)/2.;
    /// Compute average Z shift for all daughters within new cave.
    Double_t originZShift = sumOriginZ/nDaughters;
    LOG(INFO) << "Modify cave to dR = " << maxDR << " and to dZ = " << maxZ;
    caveShape->SetTubeDimensions( 0., maxDR, newCaveDZ );
    /// Compute new bounding box.
    caveShape->ComputeBBox();


    /// Update global translation of all daughters by what we picked up before...
    node = nullptr;
    TIter nextNode(caveNode->GetNodes());
    while( node = (TGeoNode*)nextNode() )
    {
      /// Get the original z position...
      const Double_t* trans = node->GetMatrix()->GetTranslation();
      node->GetMatrix()->SetDz( originZShift + trans[2] );
    }
  } /// End if(mOptimize)
}

Cave::Cave() : FairDetector() {}
Cave::~Cave() = default;
Cave::Cave(const char* name, const char* Title, bool optimize) 
  : FairDetector(name, Title, -1),
    mOptimize(optimize)
{
}
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
