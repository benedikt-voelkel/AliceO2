#include "O2G4EventAction.h"

#include <G4Event.hh>
#include <G4TrajectoryContainer.hh>
#include <G4Trajectory.hh>
#include <G4VVisManager.hh>
#include <G4UImanager.hh>
#include <Randomize.hh>

#include <math.h>


//
// public methods
//

O2G4EventAction::O2G4EventAction() : G4UserEventAction(), O2SimInterface(), fTrackingAction(nullptr)
{}

//_____________________________________________________________________________
void O2G4EventAction::Initialize()
{
/// Cache thread-local pointers
  // NOTE That needs to be changed
  // Provide the user with an intreface to obtain the other actions to pass them
  // as memebrs to other actions
  //fTrackingAction = TG4TrackingAction::Instance();
}

//_____________________________________________________________________________
void O2G4EventAction::BeginOfEventAction(const G4Event* event)
{
/// Called by G4 kernel at the beginning of event.

  // reset the tracks counters
  fTrackingAction->PrepareNewEvent();

  // save the event random number status per event
  /*
  if ( fSaveRandomStatus) {
    G4UImanager::GetUIpointer()->ApplyCommand("/random/saveThisEvent");
    if (VerboseLevel() > 0)
      G4cout << "Saving random status: " << G4endl;
      CLHEP::HepRandom::showEngineStatus();
      G4cout << G4endl;
  }
  */

  G4cout << ">>> Event " << event->GetEventID() << G4endl;
  //fTimer.Start();
}

//_____________________________________________________________________________
void O2G4EventAction::EndOfEventAction(const G4Event* event)
{
/// Called by G4 kernel at the end of event.

  // finish the last primary track of the current event
  fTrackingAction->FinishPrimaryTrack();

  // User SDs finish event
  if ( TG4SDServices::Instance()->GetUserSDs() ) {
    for (auto& userSD : (*TG4SDServices::Instance()->GetUserSDs()) ) {
      userSD->EndOfEvent();
    }
  }

  // print time
  G4cout << ">>> End of Event " << event->GetEventID() << G4endl;
  //fTimer.Stop();
  //fTimer.Print();
  /*
  if ( fPrintMemory ) {
    ProcInfo_t procInfo;
    gSystem->GetProcInfo(&procInfo);
    G4cout << "Current memory usage: resident "
           << procInfo.fMemResident << ", virtual " << procInfo.fMemVirtual << G4endl;
  }
  */
}
