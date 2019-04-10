#include <iostream>

#ifdef G4MULTITHREADED
#include <G4MTRunManager.hh>
#endif
#include <G4TransportationManager.hh>
#include "FTFP_BERT.hh"

#include "TGeoManager.h"

//#include "O2G4DetectorConstruction.h"

#include "TG4RootDetectorConstruction.h"
#include "TG4RootNavigator.h"

#include "O2G4PrimaryGeneratorAction.h"
#include "O2G4RunAction.h"
#include "O2G4EventAction.h"
#include "O2G4TrackingAction.h"
#include "O2G4SteppingAction.h"
#include "O2G4DefaultRunConfiguration.h"
#include "O2G4WorkerInitialization.h"


O2G4DefaultRunConfiguration::O2G4DefaultRunConfiguration()
  : VO2G4RunConfiguration(), fPhysicsList(new FTFP_BERT())
{
  if(!gGeoManager) {
    fGeoManager = new TGeoManager("TGeo", "Root geometry manager");
  } else {
    fGeoManager = gGeoManager;
  }
  fDetectorConstruction = new TG4RootDetectorConstruction(fGeoManager);
  std::cerr << "O2G4DefaultRunConfiguration constructed" << std::endl;
}

EExitStatus O2G4DefaultRunConfiguration::Initialize()
{
  fDetectorConstruction->Initialize(nullptr);
  G4int nthreads = 1;

  #ifdef G4MULTITHREADED
  nthreads = G4MTRunManager::GetMasterRunManager()->GetNumberOfThreads();
  #endif
  if (nthreads > 1) {
    gGeoManager->SetMaxThreads(nthreads);
  }
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

G4Navigator* O2G4DefaultRunConfiguration::CreateMasterNavigatorForTracking() const
{
  std::cerr << "TG4RootNavMgr has been created on worker" << std::endl;
  return new TG4RootNavigator(fDetectorConstruction);
}

G4Navigator* O2G4DefaultRunConfiguration::CreateWorkerNavigatorForTracking() const
{
  // TODO Add mutex?!
  std::cerr << "TG4RootNavMgr has been created on worker" << std::endl;
  return new TG4RootNavigator(fDetectorConstruction);
}
