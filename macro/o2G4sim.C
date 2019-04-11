// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "build_geometry.C"
#if !defined(__CLING__) || defined(__ROOTCLING__)
#include <Generators/PrimaryGenerator.h>
#include <Generators/GeneratorFactory.h>
#include <Generators/PDG.h>
#include "SimulationDataFormat/MCEventHeader.h"
#include <SimConfig/SimConfig.h>
#include <SimConfig/ConfigurableParam.h>
#include <CommonUtils/RngHelper.h>
#include <TStopwatch.h>
#include <memory>
#include "DataFormatsParameters/GRPObject.h"
#include "FairParRootFileIo.h"
#include "FairSystemInfo.h"
#include <SimSetup/SimSetup.h>
#include <Steer/O2G4RunSim.h>
#include <unistd.h>
#include <sstream>

#include "DetectorsBase/Detector.h"

#include "Steer/O2G4RunSim.h"
#include "Steer/O2G4MCApplication.h"
#include "Simulation/O2G4RunManager.h"
#include "Simulation/O2G4DefaultRunConfiguration.h"
#endif

FairRunSim* o2sim_init(FairRunSim* run)
{
  auto& confref = o2::conf::SimConfig::Instance();
  // we can read from CCDB (for the moment faking with a TFile)
  // o2::conf::ConfigurableParam::fromCCDB("params_ccdb.root", runid);

  // update the parameters from stuff given at command line
  o2::conf::ConfigurableParam::updateFromString(confref.getKeyValueString());
  // write the configuration file
  o2::conf::ConfigurableParam::writeINI("o2sim_configuration.ini");

  // we can update the binary CCDB entry something like this ( + timestamp key )
  // o2::conf::ConfigurableParam::toCCDB("params_ccdb.root");

  // set seed
  auto seed = o2::utils::RngHelper::setGRandomSeed(confref.getStartSeed());
  LOG(INFO) << "RNG INITIAL SEED " << seed;

  //auto genconfig = confref.getGenerator();
  //FairRunSim* run = new o2::steer::O2G4RunSim();
  //run->SetImportTGeoToVMC(false); // do not import TGeo to VMC since the latter is built together with TGeo
  //run->SetSimSetup([confref]() { o2::SimSetup::setup(confref.getMCEngine().c_str()); });



  auto header = new o2::dataformats::MCEventHeader();
  run->SetMCEventHeader(header);

  // construct geometry / including magnetic field
  build_geometry(run);

  // setup generator
  auto embedinto_filename = confref.getEmbedIntoFileName();
  auto primGen = new o2::eventgen::PrimaryGenerator();

  if (!embedinto_filename.empty()) {
    primGen->embedInto(embedinto_filename);
  }
  /*
  if (!asservice) {
    o2::eventgen::GeneratorFactory::setPrimaryGenerator(confref, primGen);
  }
  */

  run->SetGenerator(primGen);

  // Timer
  TStopwatch timer;
  timer.Start();

  // run init
  run->Init();
  auto g4Config = new O2G4DefaultRunConfiguration();
  auto runManager = new O2G4RunManager(g4Config);
  const TObjArray* modArr = run->GetListOfModules();
  TIter next(modArr);
  FairModule* module = nullptr;
  while ((module = (FairModule*)next())) {
    auto o2Det = dynamic_cast<o2::base::Detector*>(module);
    if(!o2Det) {
      LOG(WARNING) << "Could not cast to o2::base::Detector";
      continue;
    }
    LOG(INFO) << "Added potential SD" << o2Det->GetName();
    g4Config->AddPotentialSD(o2Det);
  }
  runManager->Initialize();
  finalize_geometry(run);


  std::stringstream geomss;
  geomss << "O2geometry";
  geomss << ".root";
  gGeoManager->Export(geomss.str().c_str());
  std::time_t runStart = std::time(nullptr);

  // runtime database
  bool kParameterMerged = true;
  auto rtdb = run->GetRuntimeDb();
  auto parOut = new FairParRootFileIo(kParameterMerged);

  std::stringstream s2;
  s2 << confref.getOutPrefix();
  s2 << "_par.root";
  std::string parfilename = s2.str();
  parOut->open(parfilename.c_str());
  rtdb->setOutput(parOut);
  rtdb->saveOutput();
  rtdb->print();
  o2::PDG::addParticlesToPdgDataBase(0);

  {
    // store GRPobject
    o2::parameters::GRPObject grp;
    grp.setRun(run->GetRunId());
    grp.setTimeStart(runStart);
    grp.setTimeEnd(std::time(nullptr));
    TObjArray* modArr = run->GetListOfModules();
    TIter next(modArr);
    FairModule* module = nullptr;
    while ((module = (FairModule*)next())) {
      o2::base::Detector* det = dynamic_cast<o2::base::Detector*>(module);
      if (!det) {
        continue; // not a detector
      }
      if (det->GetDetId() < o2::detectors::DetID::First) {
        continue; // passive
      }
      if (det->GetDetId() > o2::detectors::DetID::Last) {
        continue; // passive
      }
      grp.addDetReadOut(o2::detectors::DetID(det->GetDetId()));
    }
    grp.print();
    printf("VMC: %p\n", TVirtualMC::GetMC());
    auto field = dynamic_cast<o2::field::MagneticField*>(run->GetField());
    if (field) {
      o2::units::Current_t currDip = field->getCurrentDipole();
      o2::units::Current_t currL3 = field->getCurrentSolenoid();
      grp.setL3Current(currL3);
      grp.setDipoleCurrent(currDip);
    }
    // save
    std::string grpfilename = confref.getOutPrefix() + "_grp.root";
    TFile grpF(grpfilename.c_str(), "recreate");
    grpF.WriteObjectAny(&grp, grp.Class(), "GRP");
  }
  // todo: save beam information in the grp

  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();

  // extract max memory usage for init
  FairSystemInfo sysinfo;
  LOG(INFO) << "Init: Real time " << rtime << " s, CPU time " << ctime << "s";
  LOG(INFO) << "Init: Memory used " << sysinfo.GetMaxMemory() << " MB";

  timer.Reset();

  timer.Start();

  //run->Run(confref.getNEvents());

  runManager->BeamOn(confref.getNEvents());

  // Finish
  timer.Stop();
  rtime = timer.RealTime();
  ctime = timer.CpuTime();


  LOG(INFO) << "Macro finished succesfully.";
  LOG(INFO) << "Real time " << rtime << " s, CPU time " << ctime << "s";
  LOG(INFO) << "Memory used " << sysinfo.GetMaxMemory() << " MB";


  return run;
}



// asservice: in a parallel device-based context?
void o2G4sim()
{
  auto run = new o2::steer::O2G4RunSim();
  o2sim_init(run);
  delete run;
}
