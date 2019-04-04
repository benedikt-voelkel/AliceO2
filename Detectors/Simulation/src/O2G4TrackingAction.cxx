#include "O2G4TrackingAction.h"

#include <G4Track.hh>
#include <G4TrackVector.hh>
#include <G4TrackingManager.hh>
#include <G4UImanager.hh>

O2G4TrackingAction::O2G4TrackingAction() : G4UserTrackingAction(), O2SimInterface()
{}

//_____________________________________________________________________________
void O2G4TrackingAction::Initialize()
{
/// Cache thread-local pointers
  return;
}


//_____________________________________________________________________________
void O2G4TrackingAction::PreUserTrackingAction(const G4Track* track)
{
/// Called by G4 kernel before starting tracking.
  return;
}

//_____________________________________________________________________________
void O2G4TrackingAction::PostUserTrackingAction(const G4Track* track)
{
/// Called by G4 kernel after finishing tracking.
  return;
}
