#include <G4Track.hh>
#include <G4TrackVector.hh>
#include <G4TrackingManager.hh>
#include <G4UImanager.hh>

#include "O2G4TrackingAction.h"

O2G4TrackingAction::O2G4TrackingAction() : G4UserTrackingAction(), O2SimInterface()
{
  std::cerr << "Constructed O2G4TrackingAction" << std::endl;
}

//_____________________________________________________________________________
EExitStatus O2G4TrackingAction::Initialize()
{
/// Cache thread-local pointers
  return O2SimInterface::Initialize();
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
