#include <G4TransportationManager.hh>
#include <G4PropagatorInField.hh>
#include <G4EventManager.hh>
#include <G4TrackingManager.hh>
#include <G4SteppingManager.hh>

#include "O2G4WorkerInitialization.h"
#include "O2G4RunManager.h"
#include "VO2G4RunConfiguration.h"

/*
#include <G4AutoLock.hh>

#ifdef G4MULTITHREADED
namespace {
  // Mutex to lock MCApplication::FinishRun
  G4Mutex finishRunMutex = G4MUTEX_INITIALIZER;
  // Mutex to lock deleting MC application
  // G4Mutex deleteMutex = G4MUTEX_INITIALIZER;
  // Mutex to lock FinishRun & deleting MC application
  G4Mutex stopWorkerMutex = G4MUTEX_INITIALIZER;
}
#endif
*/

//_____________________________________________________________________________
O2G4WorkerInitialization::O2G4WorkerInitialization(VO2G4RunConfiguration* runConfiguration)
  : G4UserWorkerInitialization(), fRunConfiguration(runConfiguration)
{
  std::cerr << "Constructed O2G4WorkerInitialization" << std::endl;
/// Standard constructor
}

//_____________________________________________________________________________
O2G4WorkerInitialization::~O2G4WorkerInitialization()
{
/// Destructor
}

//
// public methods
//

void O2G4WorkerInitialization::WorkerInitialize() const
{

}

void O2G4WorkerInitialization::WorkerStart() const
{

  auto navigator = fRunConfiguration->CreateWorkerNavigatorForTracking();
  if(!navigator) {
    G4Exception("O2G4WorkerInitialization::WorkerStart",
                "Run0012", FatalException,
                "No navigator could be created created");
    // For now let the user take care of this.
    // TODO Provide interface in VO2G4RunConfiguration to register G4Navigators to be deleted
    //G4TransportationManager::GetTransportationManager()->SetNavigatorForTracking(navigator);


  }
  auto runMgr = static_cast<O2G4RunManager*>(G4RunManager::GetRunManager());
  runMgr->RegisterNavigator(navigator);
  G4TransportationManager *trMgr = G4TransportationManager::GetTransportationManager();
  trMgr->SetNavigatorForTracking(navigator);
  G4FieldManager *fieldMgr = trMgr->GetPropagatorInField()->GetCurrentFieldManager();
  if(fieldMgr) {
    delete trMgr->GetPropagatorInField();
  }
  trMgr->SetPropagatorInField(new G4PropagatorInField(navigator, fieldMgr));
  trMgr->ActivateNavigator(navigator);
  G4EventManager::GetEventManager()->GetTrackingManager()->GetSteppingManager()->SetNavigator(navigator);
}

//_____________________________________________________________________________
void O2G4WorkerInitialization::WorkerRunStart() const
{
/// Call post initialization on workers
  return;
}


//_____________________________________________________________________________
void O2G4WorkerInitialization::WorkerRunEnd() const
{
// This method is called for each thread, when the local event loop has
// finished but before the synchronization over threads.
  return;
}

//_____________________________________________________________________________
void O2G4WorkerInitialization::WorkerStop() const
{
/// This method is called once at the end of simulation job.
/// It implements a clean up action, which is the clean-up of MC application
/// in our case.
  return;
}
