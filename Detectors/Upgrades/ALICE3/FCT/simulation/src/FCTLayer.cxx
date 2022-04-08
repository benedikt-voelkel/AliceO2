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

/// \file FCTLayer.cxx
/// \brief Implementation of the FCTLayer class
/// \author Mario Sitta <sitta@to.infn.it>
/// \author Chinorat Kobdaj (kobdaj@g.sut.ac.th)

#include "FCTSimulation/FCTLayer.h"
#include "FCTBase/GeometryTGeo.h"
#include "FCTSimulation/Detector.h"

#include "FairLogger.h" // for LOG

#include <TGeoManager.h>        // for TGeoManager, gGeoManager
#include <TGeoMatrix.h>         // for TGeoCombiTrans, TGeoRotation, etc
#include <TGeoTube.h>           // for TGeoTube, TGeoTubeSeg
#include <TGeoVolume.h>         // for TGeoVolume, TGeoVolumeAssembly
#include <TGeoCompositeShape.h> // for TGeoCompositeShape
#include "TMathBase.h"          // for Abs
#include <TMath.h>              // for Sin, RadToDeg, DegToRad, Cos, Tan, etc

#include <cstdio> // for snprintf

class TGeoMedium;

using namespace TMath;
using namespace o2::fct;
using namespace o2::itsmft;

ClassImp(FCTLayer);

FCTLayer::~FCTLayer() = default;

FCTLayer::FCTLayer(Int_t layerDirection, Int_t layerNumber, std::string layerName, Float_t z, Float_t rIn, Float_t rOut_SideL, Float_t Layerx2X0, Int_t type)
{
  // Creates a simple parametrized EndCap layer covering the given
  // pseudorapidity range at the z layer position
  mDirection = layerDirection;
  mLayerNumber = layerNumber;
  mLayerName = layerName;
  mZ = layerDirection ? std::abs(z) : -std::abs(z);
  mx2X0 = Layerx2X0;
  mInnerRadius = rIn;
  mType = type;
  if(type == 0) { // Disk
    mOuterRadius = rOut_SideL;
  } else if(type == 1) { // Square
    mSideLength = rOut_SideL;
  }
  auto Si_X0 = 9.5;
  mChipThickness = Layerx2X0 * Si_X0;
  if(mType == 0){
    LOG(info) << "Creating FCT Disk Layer " << mLayerNumber << " ; direction " << mDirection;
    LOG(info) << "   Using silicon X0 = " << Si_X0 << " to emulate layer radiation length.";
    LOG(info) << "   Layer z = " << mZ << " ; R_in = " << mInnerRadius << " ; R_out = " << mOuterRadius << " ; x2X0 = " << mx2X0 << " ; ChipThickness = " << mChipThickness;
  } else if(mType == 1){
    LOG(info) << "Creating FCT Square Layer " << mLayerNumber << " ; direction " << mDirection;
    LOG(info) << "   Using silicon X0 = " << Si_X0 << " to emulate layer radiation length.";
    LOG(info) << "   Layer z = " << mZ << " ; R_in = " << mInnerRadius << " ; L_side = " << mSideLength << " ; x2X0 = " << mx2X0 << " ; ChipThickness = " << mChipThickness;
  }
}

void FCTLayer::createLayer(TGeoVolume* motherVolume)
{
  if(mType == 0) {
    createDiskLayer(motherVolume);
  } else if(mType == 1) {
    createSquareLayer(motherVolume);
  }
  return;
}


void FCTLayer::createDiskLayer(TGeoVolume* motherVolume){
 if (mLayerNumber >= 0) {
    // Create tube, set sensitive volume, add to mother volume

    std::string chipName = o2::fct::GeometryTGeo::getFCTChipPattern() + std::to_string(mLayerNumber),
                sensName = Form("%s_%d_%d", GeometryTGeo::getFCTSensorPattern(), mDirection, mLayerNumber);
    TGeoTube* sensor = new TGeoTube(mInnerRadius, mOuterRadius, mChipThickness / 2);
    TGeoTube* chip = new TGeoTube(mInnerRadius, mOuterRadius, mChipThickness / 2);
    TGeoTube* layer = new TGeoTube(mInnerRadius, mOuterRadius, mChipThickness / 2);

    TGeoMedium* medSi = gGeoManager->GetMedium("FCT_SI$");
    TGeoMedium* medAir = gGeoManager->GetMedium("FCT_AIR$");

    TGeoVolume* sensVol = new TGeoVolume(sensName.c_str(), sensor, medSi);
    TGeoVolume* chipVol = new TGeoVolume(chipName.c_str(), chip, medSi);
    TGeoVolume* layerVol = new TGeoVolume(mLayerName.c_str(), layer, medAir);

    LOG(info) << "Inserting " << sensVol->GetName() << " inside " << chipVol->GetName();
    chipVol->AddNode(sensVol, 1, nullptr);

    LOG(info) << "Inserting " << chipVol->GetName() << " inside " << layerVol->GetName();
    layerVol->AddNode(chipVol, 1, nullptr);

    // Finally put everything in the mother volume
    auto* FwdDiskRotation = new TGeoRotation("FwdDiskRotation", 0, 0, 180);
    auto* FwdDiskCombiTrans = new TGeoCombiTrans(0, 0, mZ, FwdDiskRotation);

    LOG(info) << "Inserting " << layerVol->GetName() << " inside " << motherVolume->GetName();
    motherVolume->AddNode(layerVol, 1, FwdDiskCombiTrans);

    return;
  }
}

void FCTLayer::createSquareLayer(TGeoVolume* motherVolume){
  if (mLayerNumber >= 0) {

    LOG(info) << "Constructing a layer and adding it to the motherVolume";

    std::string chipName = o2::fct::GeometryTGeo::getFCTChipPattern() + std::to_string(mLayerNumber),
                sensName = Form("%s_%d_%d", GeometryTGeo::getFCTSensorPattern(), mDirection, mLayerNumber);
    TGeoBBox* sensorBox = new TGeoBBox("SensorBox", mSideLength, mSideLength, mChipThickness / 2);
    TGeoBBox* chipBox = new TGeoBBox("ChipBox", mSideLength, mSideLength, mChipThickness / 2);
    TGeoBBox* layerBox = new TGeoBBox("LayerBox", mSideLength, mSideLength, mChipThickness / 2);

    TGeoTube* sensorCutout = new TGeoTube("SensorTube", 0., mInnerRadius, mChipThickness / 2);
    TGeoTube* chipCutout = new TGeoTube("ChipTube", 0., mInnerRadius, mChipThickness / 2);
    TGeoTube* layerCutout = new TGeoTube("LayerTube", 0., mInnerRadius, mChipThickness / 2);

    TGeoCompositeShape* SensorComp = new TGeoCompositeShape("SensorComp", "SensorBox - SensorTube");
    TGeoCompositeShape* ChipComp = new TGeoCompositeShape("SensorComp", "ChipBox - ChipTube");
    TGeoCompositeShape* LayerComp = new TGeoCompositeShape("SensorComp", "LayerBox - LayerTube");

    TGeoMedium* medSi = gGeoManager->GetMedium("FCT_SI$");
    TGeoMedium* medAir = gGeoManager->GetMedium("FCT_AIR$");

    TGeoVolume* sensVol = new TGeoVolume(sensName.c_str(), SensorComp, medSi);
    TGeoVolume* chipVol = new TGeoVolume(chipName.c_str(), ChipComp, medSi);
    TGeoVolume* layerVol = new TGeoVolume(mLayerName.c_str(), LayerComp, medAir);

    LOG(info) << "Inserting " << sensVol->GetName() << " inside " << chipVol->GetName();
    chipVol->AddNode(sensVol, 1, nullptr);

    LOG(info) << "Inserting " << chipVol->GetName() << " inside " << layerVol->GetName();
    layerVol->AddNode(chipVol, 1, nullptr);

    // Finally put everything in the mother volume
    auto* FwdLayerRotation = new TGeoRotation("FwdDiskRotation", 0, 0, 180);
    auto* FwdLayerCombiTrans = new TGeoCombiTrans(0, 0, mZ, FwdLayerRotation);

    LOG(info) << "Inserting " << layerVol->GetName() << " inside " << motherVolume->GetName();
    motherVolume->AddNode(layerVol, 1, FwdLayerCombiTrans);
    
    return;
  }
}