#include <G4UImessenger.hh>

#include "VO2G4RunConfiguration.h"
/*
#ifdef USE_VGM
#include "TG4VGMMessenger.h"
#include <XmlVGM/AGDDExporter.h>
#include <XmlVGM/GDMLExporter.h>
#endif
*/

VO2G4RunConfiguration::VO2G4RunConfiguration()
  : G4VUserActionInitialization(), O2SimInterface()
{}

//_____________________________________________________________________________
void VO2G4RunConfiguration::BuildForMaster() const
{
/// Build user actions defined on master thread
  G4UserRunAction* runAction = CreateMasterRunAction();
  if ( runAction ) {
    SetUserAction(runAction);
  }
}

//_____________________________________________________________________________
void VO2G4RunConfiguration::Build() const
{
/// Build user actions defined on worker threads

  // Create actions on workers
  // That one is required
  auto primaryGeneratorAction = CreatePrimaryGeneratorAction();
  if(!primaryGeneratorAction) {
    G4Exception("VO2G4RunConfiguration::Build", "Run0031",
                FatalException, "Cannot find G4G4VUserPrimaryGeneratorAction.");

  }
  SetUserAction(primaryGeneratorAction);
  // Create actions (without messengers) which were not yet created
  // and set them to G4RunManager
  #ifdef G4MULTITHREADED
    auto runAction = CreateWorkerRunAction();
  #else
    auto runAction = CreateMasterRunAction();
  #endif
  auto eventAction = CreateEventAction();
  auto trackingAction = CreateTrackingAction();
  auto steppingAction = CreateSteppingAction();
  auto stackingAction = CreateStackingAction();

  // Set already created actions (with messengers) to G4RunManager
  if ( eventAction ) {
    SetUserAction(eventAction);
  }
  if ( trackingAction ) {
    SetUserAction(trackingAction);
  }
  if ( steppingAction ) {
    SetUserAction(steppingAction);
  }
  if ( stackingAction ) {
    SetUserAction(stackingAction);
  }
  if ( runAction ) {
    SetUserAction(runAction);
  }
}


EExitStatus VO2G4RunConfiguration::Initialize()
{
  return O2SimInterface::Initialize();
}
