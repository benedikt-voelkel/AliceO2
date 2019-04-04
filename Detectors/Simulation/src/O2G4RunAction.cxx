#include "O2G4RunAction.h"

#include <G4Run.hh>
#include <G4AutoLock.hh>
#include <G4Timer.hh>

// mutex in a file scope

#ifdef G4MULTITHREADED
namespace {
  //Mutex to lock master application when merging data
  G4Mutex mergeMutex = G4MUTEX_INITIALIZER;
}
#endif

//_____________________________________________________________________________
O2G4RunAction::O2G4RunAction()
  : G4UserRunAction(),
    O2SimInterface(),
    fTimer(new G4Timer),
    fRunID(-1)
{
}

//
// public methods
//

//_____________________________________________________________________________
void O2G4RunAction::BeginOfRunAction(const G4Run* run)
{
/// Called by G4 kernel at the beginning of run.
  fRunID++;
  G4cout << "### Run " << run->GetRunID() << " start." << G4endl;
  fTimer->Start();
}

//_____________________________________________________________________________
void O2G4RunAction::EndOfRunAction(const G4Run* run)
{
/// Called by G4 kernel at the end of run.

/*
#ifdef G4MULTITHREADED
  // Merge user application data
  G4AutoLock lm(&mergeMutex);
  TGeant4::MasterApplicationInstance()->Merge(TVirtualMCApplication::Instance());
  lm.unlock();
#endif
*/

  /*
  if ( fCrossSectionManager.IsMakeHistograms() ) {
    fCrossSectionManager.MakeHistograms();
  }
  */

  fTimer->Stop();
  G4cout << "Time of this run:   " << *fTimer << G4endl;
  G4cout << "Number of events processed: " << run->GetNumberOfEvent() << G4endl;
}
