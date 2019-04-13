#include <iostream>

#ifdef G4MULTITHREADED
#include <G4MTRunManager.hh>
#endif
#include <G4TransportationManager.hh>
#include "FTFP_BERT.hh"
#include "QGSP_FTFP_BERT.hh"

#include "TGeoManager.h"

#include "FairModule.h"

//#include "O2G4DetectorConstruction.h"

#include "TG4RootNavigator.h"

#include "O2G4DetectorConstruction.h"
#include "O2G4PrimaryGeneratorAction.h"
#include "O2G4RunAction.h"
#include "O2G4EventAction.h"
#include "O2G4TrackingAction.h"
#include "O2G4SteppingAction.h"
#include "O2G4DefaultRunConfiguration.h"
#include "O2G4WorkerInitialization.h"


O2G4DefaultRunConfiguration::O2G4DefaultRunConfiguration()
  : VO2G4RunConfiguration(), fPhysicsList(new QGSP_FTFP_BERT())
{
  if(!gGeoManager) {
    fGeoManager = new TGeoManager("TGeo", "Root geometry manager");
  } else {
    fGeoManager = gGeoManager;
  }
  fDetectorConstruction = new O2G4DetectorConstruction(fGeoManager);
  std::cerr << "O2G4DefaultRunConfiguration constructed" << std::endl;
}

EExitStatus O2G4DefaultRunConfiguration::Initialize()
{
  return VO2G4RunConfiguration::Initialize();
}

G4VUserDetectorConstruction* O2G4DefaultRunConfiguration::CreateDetectorConstruction() const
{
  return fDetectorConstruction;
}

G4VUserPhysicsList* O2G4DefaultRunConfiguration::CreatePhysicsList() const
{
  return fPhysicsList;
}

G4VUserPrimaryGeneratorAction* O2G4DefaultRunConfiguration::CreatePrimaryGeneratorAction() const
{
  return new O2G4PrimaryGeneratorAction();
}

G4UserRunAction* O2G4DefaultRunConfiguration::CreateMasterRunAction() const
{
  return new O2G4RunAction();
}

G4UserRunAction* O2G4DefaultRunConfiguration::CreateWorkerRunAction() const
{
  return new O2G4RunAction();
}

G4UserEventAction* O2G4DefaultRunConfiguration::CreateEventAction() const
{
  return new O2G4EventAction();
}

G4UserTrackingAction* O2G4DefaultRunConfiguration::CreateTrackingAction() const
{
  return new O2G4TrackingAction();
}

G4UserSteppingAction* O2G4DefaultRunConfiguration::CreateSteppingAction() const
{
  return new O2G4SteppingAction();
}

G4UserStackingAction* O2G4DefaultRunConfiguration::CreateStackingAction() const
{
  return nullptr;
}

G4UserWorkerInitialization* O2G4DefaultRunConfiguration::CreateWorkerInitialization()
{
  return new O2G4WorkerInitialization(this);
}

G4Navigator* O2G4DefaultRunConfiguration::CreateNavigatorForTracking() const
{
  std::cerr << "TG4RootNavigator has been created" << std::endl;
  return new TG4RootNavigator(fDetectorConstruction);
}
