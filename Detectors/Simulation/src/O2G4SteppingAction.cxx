#include <G4Track.hh>

#include "O2G4SteppingAction.h"

//_____________________________________________________________________________
O2G4SteppingAction::O2G4SteppingAction()
  : G4UserSteppingAction(), O2SimInterface(),fLoopStepCounter(0), fMaxNofSteps(kMaxNofSteps)
{
  std::cerr << "Constructed O2G4SteppingAction" << std::endl;
}

//
// private methods
//

//_____________________________________________________________________________
void O2G4SteppingAction::ProcessTrackIfLooping(const G4Step* step)
{
/// Stop track if maximum number of steps has been reached.

  G4Track* track = step->GetTrack();
  G4int stepNumber = track->GetCurrentStepNumber();

  if ( fLoopStepCounter ) {
     if ( stepNumber == 1 ) {
        // reset parameters at beginning of tracking
        fLoopStepCounter = 0;
                  // necessary in case particle has reached fMaxNofSteps
                // but has stopped before processing kMaxNofLoopSteps
     }
     else {
      // count steps after detecting looping track
      fLoopStepCounter++;
      if ( fLoopStepCounter == kMaxNofLoopSteps ) {

         // stop the looping track
         track->SetTrackStatus(fStopAndKill);

         // reset parameters back
         fLoopStepCounter = 0;
      }
    }
  }
  else if ( stepNumber > fMaxNofSteps ) {


    // start looping counter
    fLoopStepCounter++;
  }
}

//
// protected methods
//

//_____________________________________________________________________________
void O2G4SteppingAction::PrintTrackInfo(const G4Track* track) const
{
/// Print the track info,
/// taken from private G4TrackingManager::Verbose(),
/// and the standard header for verbose tracking,
/// taken from G4SteppingVerbose::TrackingStarted().

  // print track info
  G4cout << G4endl;
  G4cout << "*******************************************************"
         << "**************************************************"
         << G4endl;
  G4cout << "* G4Track Information: "
         << "  Particle = " << track->GetDefinition()->GetParticleName()
         << ","
         << "   Track ID = " << track->GetTrackID()
         << ","
         << "   Parent ID = " << track->GetParentID()
         << G4endl;
  G4cout << "*******************************************************"
         << "**************************************************"
         << G4endl;
  G4cout << G4endl;

  // print header
#ifdef G4_USE_G4BESTUNIT_FOR_VERBOSE
    G4cout << std::setw( 5) << "Step#"  << " "
           << std::setw( 8) << "X"      << "     "
           << std::setw( 8) << "Y"      << "     "
           << std::setw( 8) << "Z"      << "     "
           << std::setw( 9) << "KineE"  << "     "
           << std::setw( 8) << "dE"     << "     "
           << std::setw(12) << "StepLeng"   << " "
           << std::setw(12) << "TrackLeng"  << " "
           << std::setw(12) << "NextVolume" << " "
           << std::setw( 8) << "ProcName"   << G4endl;
#else
    G4cout << std::setw( 5) << "Step#"      << " "
           << std::setw( 8) << "X(mm)"      << " "
           << std::setw( 8) << "Y(mm)"      << " "
           << std::setw( 8) << "Z(mm)"      << " "
           << std::setw( 9) << "KinE(MeV)"  << " "
           << std::setw( 8) << "dE(MeV)"    << " "
           << std::setw( 8) << "StepLeng"   << " "
           << std::setw( 9) << "TrackLeng"  << " "
           << std::setw(11) << "NextVolume" << " "
           << std::setw( 8) << "ProcName"   << G4endl;
#endif
}

//
// public methods
//

//_____________________________________________________________________________
EExitStatus O2G4SteppingAction::Initialize()
{
  return O2SimInterface::Initialize();
}

//_____________________________________________________________________________
void O2G4SteppingAction::UserSteppingAction(const G4Step* step)
{
/// Called by G4 kernel at the end of each step.
/// This method should not be overridden in a Geant4 VMC user class;
/// there is defined O2G4SteppingAction(const G4Step* step) method
/// for this purpose.

  // stop track if maximum number of steps has been reached
  ProcessTrackIfLooping(step);

  // update Root track if collecting tracks is activated
  //if ( fCollectTracks )
    //fGeoTrackManager.UpdateRootTrack(step);

  // save secondaries
  /*
  if ( fTrackManager->GetTrackSaveControl() == kSaveInStep ) {
    fTrackManager
      ->SaveSecondaries(step->GetTrack(), step->GetSecondary());
  }

  // apply special controls if init step or if crossing geometry border
  if ( step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary &&
       fSpecialControls && fSpecialControls->IsApplicable() ) {

      fSpecialControls->ApplyControls();
  }
  */
}
