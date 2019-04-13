#include <G4TransportationManager.hh>
#include <G4PropagatorInField.hh>
#include <G4EventManager.hh>
#include <G4TrackingManager.hh>
#include <G4SteppingManager.hh>
#include <G4SDManager.hh>

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
  G4cout << "Constructed O2G4WorkerInitialization" << G4endl;
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

void O2G4WorkerInitialization::WorkerStart() const
{
  /// Call post initialization on workers
  auto navigator = fRunConfiguration->CreateNavigatorForTracking();
  if(!navigator) {
    G4Exception("O2G4WorkerInitialization::WorkerStart",
                "Run0012", FatalException,
                "No navigator could be created created");
  }


  auto runMgr = static_cast<O2G4RunManager*>(G4RunManager::GetRunManager());
  runMgr->SetVerboseLevel(2);
  runMgr->RegisterNavigator(navigator);
  G4TransportationManager *trMgr = G4TransportationManager::GetTransportationManager();
  trMgr->SetNavigatorForTracking(navigator);
  // TODO Taken from TG4: Why deleting and re-constructing the G4FieldManager?
  G4FieldManager* fieldMgr = trMgr->GetPropagatorInField()->GetCurrentFieldManager();
  if(fieldMgr) {
    delete trMgr->GetPropagatorInField();
  }
  trMgr->SetPropagatorInField(new G4PropagatorInField(navigator, fieldMgr));
  trMgr->ActivateNavigator(navigator);

  // At this stage the G4SteppingManager has been already created in this thread
  // and has taken the default G4Navigator from the G4TransportationManager during
  // construction ==> reset that
  G4EventManager::GetEventManager()->GetTrackingManager()->GetSteppingManager()->SetNavigator(navigator);
  G4EventManager::GetEventManager()->SetVerboseLevel(2);
  G4EventManager::GetEventManager()->GetTrackingManager()->SetVerboseLevel(2);
  G4EventManager::GetEventManager()->GetTrackingManager()->GetSteppingManager()->SetVerboseLevel(2);
  G4SDManager::GetSDMpointer()->SetVerboseLevel(2);
}



//_____________________________________________________________________________
void O2G4WorkerInitialization::WorkerRunStart() const
{
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
