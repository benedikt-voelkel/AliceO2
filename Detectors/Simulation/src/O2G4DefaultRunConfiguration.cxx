#include <G4MTRunManager.hh>
#include <G4TransportationManager.hh>

#include "O2G4PrimaryGeneratorAction.h"
#include "O2G4RunActionAction.h"
#include "O2G4EventAction.h"
#include "O2G4TrackingAction.h"
#include "O2G4SteppingAction.h"

#include "O2G4WorkerInitialization.h"

//#include "O2G4DetectorConstruction.h"

#include "TG4RootDetectorConstruction.h"

#include "FTFP_BERT.hh"

#include "O2G4DefaultRunConfiguration.h"


O2G4DefaultRunConfiguration::O2G4DefaultRunConfiguration()
  : VO2G4O2G4DefaultRunConfiguration(), fGeoManager(new TGeoManager()),
    fDetectorConstruction(new TG4RootDetectorConstruction(fGeoManager)),
    fRootNavMgr(nullptr),
    fPhysicsList(new FTFP_BERT())
{
  fDetectorConstruction->ConstructRootGeometry();
  fRootNavMgr = new TG4RootNavMgr(fGeoManager, fDetectorConstruction);
}

O2G4DefaultRunConfiguration::Initialize()
{
  #ifdef G4MULTITHREADED
  fRootNavMgr->Initialize(nullptr, G4MTRunManager::GetMasterRunManager()->GetNumberOfThreads());
  #else
  fRootNavMgr->Initialize(nullptr, 1);
  #endif
  fRootNavMgr->ConnectToG4();
  return O2SimInterface::Initialize();
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
  return new O2G4RunActionAction();
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

G4UserWorkerInitialization* O2G4DefaultRunConfiguration::CreateWorkerInitialization() const
{
  return new O2G4WorkerInitialization();
}

G4Navigator* O2G4DefaultRunConfiguration::CreateMasterNavigatorForTracking() const
{
  return fRootNavMgr->GetNavigator();
}

G4Navigator* O2G4DefaultRunConfiguration::CreateWorkerNavigatorForTracking() const
{
  // TODO Add mutex?!
  auto navMgr = new TG4RootNavMgr(*fRootNavMgr);
  navMgr->ConnectToG4();
  return navMgr;
}
