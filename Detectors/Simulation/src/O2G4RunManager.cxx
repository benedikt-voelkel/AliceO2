#include <G4StateManager.hh>
#include <G4ApplicationState.hh>
#include <G4Threading.hh>
#include <G4TransportationManager.hh>

#ifdef G4VIS_USE
#include <G4VisExecutive.hh>
#endif

#include "O2G4RunManager.h"
#include "O2SimInterface.h"

//_____________________________________________________________________________
//O2G4RunManager* O2G4RunManager::fgMasterInstance = 0;


//_____________________________________________________________________________
O2G4RunManager::O2G4RunManager(VO2G4RunConfiguration* configuration)
#ifdef G4MULTITHREADED
  : G4RunManagerMT(),
#else
  : G4RunManager(),
#endif
    fRunConfiguration(configuration)
{
/// Standard constructor

  // There might be some steps the user needs to do before everything is ready
  fRunConfiguration->Initialize();

  // User actions are built from this
  SetUserInitialization(fRunConfiguration);
  // Mandatory initialization of detector construction and physics list
  SetUserInitialization(fRunConfiguration->CreateDetectorConstruction());
  SetUserInitialization(fRunConfiguration->CreatePhysicsList());

  auto navigator = fRunConfiguration->CreateMasterNavigatorForTracking();
  if(navigator) {
    G4TransportationManager::GetTransportationManager()->SetNavigatorForTracking(navigator);
    RegisterNavigator(navigator);
  }

#ifdef G4MULTITHREADED
  SetUserInitialization(fRunConfiguration->CreateWorkerInitialization());
#endif

}

void O2G4RunManager::RegisterNavigator(G4Navigator* userNavigator)
{
  fUserNavigators.emplace_back(userNavigator);
}

#undef PARENT_CLASS
