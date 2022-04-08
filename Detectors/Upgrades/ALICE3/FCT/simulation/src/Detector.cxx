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

/// \file Detector.cxx
/// \brief Implementation of the Detector class

#include "ITSMFTSimulation/Hit.h"
#include "FCTBase/GeometryTGeo.h"
#include "FCTSimulation/Detector.h"
#include "FCTSimulation/FCTLayer.h"
#include "FCTBase/FCTBaseParam.h"

#include "SimulationDataFormat/Stack.h"
#include "SimulationDataFormat/TrackReference.h"

// FairRoot includes
#include "FairDetector.h"    // for FairDetector
#include "FairLogger.h"      // for LOG, LOG_IF
#include "FairRootManager.h" // for FairRootManager
#include "FairRun.h"         // for FairRun
#include "FairRuntimeDb.h"   // for FairRuntimeDb
#include "FairVolume.h"      // for FairVolume
#include "FairRootManager.h"

#include "TGeoManager.h"     // for TGeoManager, gGeoManager
#include "TGeoTube.h"        // for TGeoTube
#include "TGeoPcon.h"        // for TGeoPcon
#include "TGeoVolume.h"      // for TGeoVolume, TGeoVolumeAssembly
#include "TString.h"         // for TString, operator+
#include "TVirtualMC.h"      // for gMC, TVirtualMC
#include "TVirtualMCStack.h" // for TVirtualMCStack
#include "TMath.h"           // for Abs, ATan, Exp, Tan
#include "TCanvas.h"
#include "TDatime.h"         // for GetDate, GetTime

#include <cstdio> // for NULL, snprintf

class FairModule;

class TGeoMedium;

class TParticle;

using namespace o2::fct;
using o2::itsmft::Hit;

//_________________________________________________________________________________________________
Detector::Detector()
  : o2::base::DetImpl<Detector>("FCT", kTRUE),
    mTrackData(),
    mHits(o2::utils::createSimVector<o2::itsmft::Hit>())
{
}

//_________________________________________________________________________________________________
void Detector::buildFCTFromFile(std::string configFileName)
{
  // Geometry description from file. One line per disk
  // z_layer r_in r_out Layerx2X0
  // This simple file reader is not failproof. Do not add empty lines!

  /*
  # Sample MFT configuration
  # z_layer    r_in    r_out   Layerx2X0   layerType (0 disks, 1 squares)
  -45.3       2.5     9.26    0.0042      1
  -46.7       2.5     9.26    0.0042      1
  -48.6       2.5     9.8     0.0042      1
  -50.0       2.5     9.8     0.0042      1
  -52.4       2.5     10.43   0.0042      1
  -53.8       2.5     10.43   0.0042      1
  -67.7       3.82    13.01   0.0042      1
  -69.1       3.82    13.01   0.0042      1
  -76.1       3.92    14.35   0.0042      1
  -77.5       3.92    14.35   0.0042      1
  */

  mLayerName.clear();
  mLayers.clear();
  mLayerID.clear();
  mLayerName.resize(1);
  mLayers.resize(1);

  LOG(info) << "Building FCT Detector: From file";
  LOG(info) << "   FCT detector configuration: " << configFileName;
  std::ifstream ifs(configFileName.c_str());
  if (!ifs.good()) {
    LOG(fatal) << " Invalid FCTBase.configFile!";
  }
  std::string tempstr;
  float z_layer, r_in, r_out_l_side, Layerx2X0;
  char delimiter;
  int layerNumber = 0;
  int layerNumberSquare = 0;
  int layerNumberDisk = 0;
  int pseumin = 3;
  int pseumax = 5;
  int layerType;
  bool r_toggle = false;



  while (std::getline(ifs, tempstr)) {
    if (tempstr[0] == '#') {
      LOG(info) << " Comment: " << tempstr;
      int loc = tempstr.find("r_toggle");
      if (loc != -1) {
        r_toggle = true;
        LOG(info) << " Comment: R toggle activated";
      }
      continue;
    }
    std::istringstream iss(tempstr);
    iss >> layerType;
    iss >> z_layer;
    if(r_toggle){
      auto pseurap_to_ang = [](float eta){return 2. * TMath::ATan(TMath::Exp(-eta));};
      r_in = TMath::Abs(z_layer) * TMath::Tan(pseurap_to_ang(pseumax));
      r_out_l_side = TMath::Abs(z_layer) * TMath::Tan(pseurap_to_ang(pseumin));
    } else {
      iss >> r_in;
      iss >> r_out_l_side;
    }
    iss >> Layerx2X0;
    

    std::string layerName = GeometryTGeo::getFCTLayerPattern() + std::string("_") + std::to_string(layerNumber);
    mLayerName[0].push_back(layerName);
    if(layerType == 0){
      LOG(info) << "Adding Disk Layer " << layerName << " at z = " << z_layer << " ; r_in = " << r_in << " ; r_out = " << r_out_l_side << " x/X0 = " << Layerx2X0;
      layerNumberDisk++;
    } else if(layerType == 1){
      LOG(info) << "Adding Square Layer " << layerName << " at z = " << z_layer << " ; r_in = " << r_in << " ; l_side = " << r_out_l_side << " x/X0 = " << Layerx2X0;
      layerNumberSquare++;
    }
    
    auto& thisLayer = mLayers[0].emplace_back(0, layerNumber, layerName, z_layer, r_in, r_out_l_side, Layerx2X0, layerType);
    layerNumber++;
  }

  mNumberOfLayers = layerNumber;
  LOG(info) << " Loaded FCT Detector with  " << mNumberOfLayers << " layers";
  LOG(info) << " Of which " << layerNumberDisk << " are disks";
  LOG(info) << " Of which " << layerNumberSquare << " are disks";
}

//_________________________________________________________________________________________________
void Detector::exportLayout()
{
  // Export FCT Layout description to file. One line per disk
  // z_layer r_in r_out Layerx2X0

  std::string configFileName = "FCT_layout.cfg";

  LOG(info) << "Exporting FCT Detector layout to " << configFileName;

  std::ofstream fOut(configFileName.c_str(), std::ios::out);
  if (!fOut) {
    printf("Cannot open file\n");
    return;
  }
  fOut << "#   z_layer   r_in   r_out_l_side   Layerx2X0" << std::endl;
  for (auto layers_dir : mLayers) {
    for (auto layer : layers_dir) {
      if(layer.getType() == 0){
        fOut << layer.getZ() << "  " << layer.getInnerRadius() << "  " << layer.getOuterRadius() << "  " << layer.getx2X0() << std::endl;
      } else if(layer.getType() == 1){
        fOut << layer.getZ() << "  " << layer.getInnerRadius() << "  " << layer.getSideLength() << "  " << layer.getx2X0() << std::endl;
      }
    }
  }
}

//_________________________________________________________________________________________________
void Detector::buildBasicFCT(const FCTBaseParam& param)
{
  // Build a basic parametrized FCT detector with nLayers equally spaced between z_first and z_first+z_length
  // Covering pseudo rapidity [etaIn,etaOut]. Silicon thinkness computed to match layer x/X0

  LOG(info) << "Building FCT Detector: Conical Telescope";

  auto z_first = param.z0;
  auto z_length = param.zLength;
  auto etaIn = param.etaIn;
  auto etaOut = param.etaOut;
  auto Layerx2X0 = param.Layerx2X0;
  mNumberOfLayers = param.nLayers;
  mLayerName.resize(2);
  mLayerName[0].resize(mNumberOfLayers);
  mLayerName[1].resize(mNumberOfLayers);
  mLayerID.clear();
  mLayers.resize(2);
  Int_t type = 0; // Disk

  for (Int_t direction : {0, 1}) {
    for (Int_t layerNumber = 0; layerNumber < mNumberOfLayers; layerNumber++) {
      std::string layerName = GeometryTGeo::getFCTLayerPattern() + std::to_string(layerNumber + mNumberOfLayers * direction);
      mLayerName[direction][layerNumber] = layerName;

      // Adds evenly spaced layers
      Float_t layerZ = z_first + (layerNumber * z_length / (mNumberOfLayers - 1)) * std::copysign(1, z_first);
      Float_t rIn = std::abs(layerZ * std::tan(2.f * std::atan(std::exp(-etaIn))));
      Float_t rOut = std::abs(layerZ * std::tan(2.f * std::atan(std::exp(-etaOut))));
      auto& thisLayer = mLayers[direction].emplace_back(direction, layerNumber, layerName, layerZ, rIn, rOut, Layerx2X0, type);
    }
  }
}

//_________________________________________________________________________________________________
void Detector::buildFCTV1()
{
  //Build FT3 detector according to
  //https://indico.cern.ch/event/992488/contributions/4174473/attachments/2168881/3661331/tracker_parameters_werner_jan_11_2021.pdf
  //Build FCT detector according to
  //<Insert link here>

  LOG(info) << "Building FCT Detector: V1";

  mNumberOfLayers = 10;
  Float_t sensorThickness = 30.e-4;
  Float_t layersx2X0 = 1.e-2;
  std::vector<std::array<Float_t, 5>> layersConfig{
    {26., .5, 3., 0.1f * layersx2X0}, // {z_layer, r_in, r_out, Layerx2X0}
    {30., .5, 3., 0.1f * layersx2X0},
    {34., .5, 3., 0.1f * layersx2X0},
    {77., 3.5, 35., layersx2X0},
    {100., 3.5, 35., layersx2X0},
    {122., 3.5, 35., layersx2X0},
    {150., 3.5, 100., layersx2X0},
    {180., 3.5, 100., layersx2X0},
    {220., 3.5, 100., layersx2X0},
    {279., 3.5, 100., layersx2X0}};

  mLayerName.resize(2);
  mLayerName[0].resize(mNumberOfLayers);
  mLayerName[1].resize(mNumberOfLayers);
  mLayerID.clear();
  mLayers.resize(2);
  Int_t type = 0; // Disk

  for (auto direction : {0, 1}) {
    for (int layerNumber = 0; layerNumber < mNumberOfLayers; layerNumber++) {
      std::string directionName = std::to_string(direction);
      std::string layerName = GeometryTGeo::getFCTLayerPattern() + directionName + std::string("_") + std::to_string(layerNumber);
      mLayerName[direction][layerNumber] = layerName;
      auto& z = layersConfig[layerNumber][0];

      auto& rIn = layersConfig[layerNumber][1];
      auto& rOut = layersConfig[layerNumber][2];
      auto& x0 = layersConfig[layerNumber][3];

      LOG(info) << "Adding Layer " << layerName << " at z = " << z;
      // Add layers
      auto& thisLayer = mLayers[direction].emplace_back(direction, layerNumber, layerName, z, rIn, rOut, x0, type);
    }
  }
}

//_________________________________________________________________________________________________
Detector::Detector(Bool_t active)
  : o2::base::DetImpl<Detector>("FCT", active),
    mTrackData(),
    mHits(o2::utils::createSimVector<o2::itsmft::Hit>())
{

  // FCT Base configuration parameters
  auto& fctBaseParam = FCTBaseParam::Instance();

  if (fctBaseParam.configFile != "") {
    LOG(info) << "FCT Geometry configuration file provided. Overriding FCTBase.geoModel configuration.";
    buildFCTFromFile(fctBaseParam.configFile);

  } else {
    switch (fctBaseParam.geoModel) {
      case Default:
        buildFCTV1(); // FCTV1
        break;
      case Telescope:
        buildBasicFCT(fctBaseParam); // BasicFCT = Parametrized telescopic detector (equidistant layers)
        break;
      default:
        LOG(fatal) << "Invalid Geometry.\n";
        break;
    }
  }
  exportLayout();
}

//_________________________________________________________________________________________________
Detector::Detector(const Detector& rhs)
  : o2::base::DetImpl<Detector>(rhs),
    mTrackData(),

    /// Container for data points
    mHits(o2::utils::createSimVector<o2::itsmft::Hit>())
{
  mLayerID = rhs.mLayerID;
  mLayerName = rhs.mLayerName;
  mNumberOfLayers = rhs.mNumberOfLayers;
}

//_________________________________________________________________________________________________
Detector::~Detector()
{

  if (mHits) {
    // delete mHits;
    o2::utils::freeSimVector(mHits);
  }
}

//_________________________________________________________________________________________________
Detector& Detector::operator=(const Detector& rhs)
{
  // The standard = operator
  // Inputs:
  //   Detector   &h the sourse of this copy
  // Outputs:
  //   none.
  // Return:
  //  A copy of the sourse hit h

  if (this == &rhs) {
    return *this;
  }

  // base class assignment
  base::Detector::operator=(rhs);

  mLayerID = rhs.mLayerID;
  mLayerName = rhs.mLayerName;
  mNumberOfLayers = rhs.mNumberOfLayers;
  mLayers = rhs.mLayers;
  mTrackData = rhs.mTrackData;

  /// Container for data points
  mHits = nullptr;

  return *this;
}

//_________________________________________________________________________________________________
void Detector::InitializeO2Detector()
{
  // Define the list of sensitive volumes
  LOG(info) << "Initialize FCT O2Detector";

  mGeometryTGeo = GeometryTGeo::Instance();

  defineSensitiveVolumes();
}

//_________________________________________________________________________________________________
Bool_t Detector::ProcessHits(FairVolume* vol)
{
  // This method is called from the MC stepping
  if (!(fMC->TrackCharge())) {
    return kFALSE;
  }

  Int_t lay = 0, volID = vol->getMCid();
  while ((lay <= mLayerID.size()) && (volID != mLayerID[lay])) {
    ++lay;
  }

  auto stack = (o2::data::Stack*)fMC->GetStack();

  bool startHit = false, stopHit = false;
  unsigned char status = 0;
  if (fMC->IsTrackEntering()) {
    status |= Hit::kTrackEntering;
  }
  if (fMC->IsTrackInside()) {
    status |= Hit::kTrackInside;
  }
  if (fMC->IsTrackExiting()) {
    status |= Hit::kTrackExiting;
  }
  if (fMC->IsTrackOut()) {
    status |= Hit::kTrackOut;
  }
  if (fMC->IsTrackStop()) {
    status |= Hit::kTrackStopped;
  }
  if (fMC->IsTrackAlive()) {
    status |= Hit::kTrackAlive;
  }

  // track is entering or created in the volume
  if ((status & Hit::kTrackEntering) || (status & Hit::kTrackInside && !mTrackData.mHitStarted)) {
    startHit = true;
  } else if ((status & (Hit::kTrackExiting | Hit::kTrackOut | Hit::kTrackStopped))) {
    stopHit = true;
  }

  // increment energy loss at all steps except entrance
  if (!startHit) {
    mTrackData.mEnergyLoss += fMC->Edep();
  }
  if (!(startHit | stopHit)) {
    return kFALSE; // do nothing
  }
  if (startHit) {
    mTrackData.mEnergyLoss = 0.;
    fMC->TrackMomentum(mTrackData.mMomentumStart);
    fMC->TrackPosition(mTrackData.mPositionStart);
    mTrackData.mTrkStatusStart = status;
    mTrackData.mHitStarted = true;
  }
  if (stopHit) {
    TLorentzVector positionStop;
    fMC->TrackPosition(positionStop);
    // Retrieve the indices with the volume path
    int chipindex = lay;

    Hit* p = addHit(stack->GetCurrentTrackNumber(), chipindex, mTrackData.mPositionStart.Vect(), positionStop.Vect(),
                    mTrackData.mMomentumStart.Vect(), mTrackData.mMomentumStart.E(), positionStop.T(),
                    mTrackData.mEnergyLoss, mTrackData.mTrkStatusStart, status);
    // p->SetTotalEnergy(vmc->Etot());

    // RS: not sure this is needed
    // Increment number of Detector det points in TParticle
    stack->addHit(GetDetId());
  }

  return kTRUE;
}

//_________________________________________________________________________________________________
void Detector::createMaterials()
{
  Int_t ifield = 2;
  Float_t fieldm = 10.0;
  o2::base::Detector::initFieldTrackingParams(ifield, fieldm);

  Float_t tmaxfdSi = 0.1;    // .10000E+01; // Degree
  Float_t stemaxSi = 0.0075; //  .10000E+01; // cm
  Float_t deemaxSi = 0.1;    // 0.30000E-02; // Fraction of particle's energy 0<deemax<=1
  Float_t epsilSi = 1.0E-4;  // .10000E+01;
  Float_t stminSi = 0.0;     // cm "Default value used"

  Float_t tmaxfdAir = 0.1;        // .10000E+01; // Degree
  Float_t stemaxAir = .10000E+01; // cm
  Float_t deemaxAir = 0.1;        // 0.30000E-02; // Fraction of particle's energy 0<deemax<=1
  Float_t epsilAir = 1.0E-4;      // .10000E+01;
  Float_t stminAir = 0.0;         // cm "Default value used"

  // AIR
  Float_t aAir[4] = {12.0107, 14.0067, 15.9994, 39.948};
  Float_t zAir[4] = {6., 7., 8., 18.};
  Float_t wAir[4] = {0.000124, 0.755267, 0.231781, 0.012827};
  Float_t dAir = 1.20479E-3;

  o2::base::Detector::Mixture(1, "AIR$", aAir, zAir, dAir, 4, wAir);
  o2::base::Detector::Medium(1, "AIR$", 1, 0, ifield, fieldm, tmaxfdAir, stemaxAir, deemaxAir, epsilAir, stminAir);

  o2::base::Detector::Material(3, "SI$", 0.28086E+02, 0.14000E+02, 0.23300E+01, 0.93600E+01, 0.99900E+03);
  o2::base::Detector::Medium(3, "SI$", 3, 0, ifield, fieldm, tmaxfdSi, stemaxSi, deemaxSi, epsilSi, stminSi);
}

//_________________________________________________________________________________________________
void Detector::EndOfEvent() { Reset(); }

//_________________________________________________________________________________________________
void Detector::Register()
{
  // This will create a branch in the output tree called Hit, setting the last
  // parameter to kFALSE means that this collection will not be written to the file,
  // it will exist only during the simulation

  if (FairRootManager::Instance()) {
    FairRootManager::Instance()->RegisterAny(addNameTo("Hit").data(), mHits, kTRUE);
  }
}

//_________________________________________________________________________________________________
void Detector::Reset()
{
  if (!o2::utils::ShmManager::Instance().isOperational()) {
    mHits->clear();
  }
}

//_________________________________________________________________________________________________
void Detector::ConstructGeometry()
{
  // Create detector materials
  createMaterials();

  // Construct the detector geometry
  createGeometry();
}

//_________________________________________________________________________________________________
void Detector::createGeometry()
{

  mGeometryTGeo = GeometryTGeo::Instance();

  TGeoVolume* volFCT = new TGeoVolumeAssembly(GeometryTGeo::getFCTVolPattern());
  TGeoVolume* volIFCT = new TGeoVolumeAssembly(GeometryTGeo::getFCTInnerVolPattern());

  LOG(info) << "GeometryBuilder::buildGeometry volume name = " << GeometryTGeo::getFCTVolPattern();

  TGeoVolume* vALIC = gGeoManager->GetVolume("barrel");
  if (!vALIC) {
    LOG(fatal) << "Could not find the top volume";
  }

  TGeoVolume* A3IPvac = gGeoManager->GetVolume("OUT_PIPEVACUUM");
  if (!A3IPvac) {
    LOG(info) << "Running simulation with no beam pipe.";
  }

  LOG(debug) << "FCT createGeometry: "
             << Form("gGeoManager name is %s title is %s", gGeoManager->GetName(), gGeoManager->GetTitle());

  if (mLayers.size() == 2) { // V1 and telescope
    if (!A3IPvac) {
      for (Int_t direction : {0, 1}) { // Backward layers at mLayers[0]; Forward layers at mLayers[1]
        std::string directionString = direction ? "Forward" : "Backward";
        LOG(info) << "Creating FCT " << directionString << " layers:";
        for (Int_t iLayer = 0; iLayer < mLayers[direction].size(); iLayer++) {
          mLayers[direction][iLayer].createLayer(volFCT);
        }
      }
      vALIC->AddNode(volFCT, 2, new TGeoTranslation(0., 30., 0.));
    } else { // If beampipe is enabled append inner disks to beampipe filling volume, this should be temporary.
      for (Int_t direction : {0, 1}) {
        std::string directionString = direction ? "Forward" : "Backward";
        LOG(info) << "Creating FCT " << directionString << " layers:";
        for (Int_t iLayer = 0; iLayer < mLayers[direction].size(); iLayer++) {
          if (iLayer < 3) {
            mLayers[direction][iLayer].createLayer(volIFCT);
          } else {
            mLayers[direction][iLayer].createLayer(volFCT);
          }
        }
      }
      A3IPvac->AddNode(volIFCT, 2, new TGeoTranslation(0., 0., 0.));
      vALIC->AddNode(volFCT, 2, new TGeoTranslation(0., 30., 0.));
    }

    for (auto direction : {0, 1}) {
      std::string directionString = direction ? "Forward" : "Backward";
      LOG(info) << "Registering FCT " << directionString << " LayerIDs:";
      for (int iLayer = 0; iLayer < mLayers[direction].size(); iLayer++) {
        auto layerID = gMC ? TVirtualMC::GetMC()->VolId(Form("%s_%d_%d", GeometryTGeo::getFCTSensorPattern(), direction, iLayer)) : 0;
        mLayerID.push_back(layerID);
        LOG(info) << " " << directionString << " layer " << iLayer << " LayerID " << layerID;
      }
    }
  }

  if (mLayers.size() == 1) { // All layers registered at mLayers[0], used when building from file
    LOG(info) << "Creating FCT layers:";
    if (A3IPvac) {
      for (Int_t iLayer = 0; iLayer < mLayers[0].size(); iLayer++) {
        if (std::abs(mLayers[0][iLayer].getZ()) < 25) {
          mLayers[0][iLayer].createLayer(volIFCT);
        } else {
          mLayers[0][iLayer].createLayer(volFCT);
        }
      }
      A3IPvac->AddNode(volIFCT, 2, new TGeoTranslation(0., 0., 0.));
      vALIC->AddNode(volFCT, 2, new TGeoTranslation(0., 30., 0.));
    } else {
      for (Int_t iLayer = 0; iLayer < mLayers[0].size(); iLayer++) {
        mLayers[0][iLayer].createLayer(volFCT);
      }
      vALIC->AddNode(volFCT, 2, new TGeoTranslation(0., 30., 0.));
    }
    LOG(info) << "Registering FCT LayerIDs:";
    for (int iLayer = 0; iLayer < mLayers[0].size(); iLayer++) {
      auto layerID = gMC ? TVirtualMC::GetMC()->VolId(Form("%s_%d_%d", GeometryTGeo::getFCTSensorPattern(), 0, iLayer)) : 0;
      mLayerID.push_back(layerID);
      LOG(info) << "  mLayerID[" << iLayer << "] = " << layerID;
    }
  }
  TCanvas *c1 = new TCanvas();
  volFCT->Draw();
  TString output_path = "/misc/alidata131/alice_u/cas/O2/Schematic_layouts/";
  TDatime* time = new TDatime();
  TString output_name = "FCT_Layout_";
  output_name.Append(Form("%d_%d.root", time->GetDate(), time->GetTime()));
  output_path.Append(output_name);
  c1->Print(output_path);
}

//_________________________________________________________________________________________________
void Detector::defineSensitiveVolumes()
{
  TGeoManager* geoManager = gGeoManager;
  TGeoVolume* v;

  TString volumeName;
  LOG(info) << "Adding FCT Sensitive Volumes";

  // The names of the FCT sensitive volumes have the format: FCTSensor_(0,1)_(0...sNumberLayers-1)
  if (mLayers.size() == 2) {
    for (Int_t direction : {0, 1}) {
      for (Int_t iLayer = 0; iLayer < mNumberOfLayers; iLayer++) {
        volumeName = o2::fct::GeometryTGeo::getFCTSensorPattern() + std::to_string(iLayer);
        v = geoManager->GetVolume(Form("%s_%d_%d", GeometryTGeo::getFCTSensorPattern(), direction, iLayer));
        LOG(info) << "Adding FCT Sensitive Volume => " << v->GetName();
        AddSensitiveVolume(v);
      }
    }
  }

  if (mLayers.size() == 1) {
    for (Int_t iLayer = 0; iLayer < mLayers[0].size(); iLayer++) {
      volumeName = o2::fct::GeometryTGeo::getFCTSensorPattern() + std::to_string(iLayer);
      v = geoManager->GetVolume(Form("%s_%d_%d", GeometryTGeo::getFCTSensorPattern(), 0, iLayer));
      LOG(info) << "Adding FCT Sensitive Volume => " << v->GetName();
      AddSensitiveVolume(v);
    }
  }
}

//_________________________________________________________________________________________________
Hit* Detector::addHit(int trackID, int detID, const TVector3& startPos, const TVector3& endPos,
                      const TVector3& startMom, double startE, double endTime, double eLoss, unsigned char startStatus,
                      unsigned char endStatus)
{
  mHits->emplace_back(trackID, detID, startPos, endPos, startMom, startE, endTime, eLoss, startStatus, endStatus);
  return &(mHits->back());
}

ClassImp(o2::fct::Detector);
