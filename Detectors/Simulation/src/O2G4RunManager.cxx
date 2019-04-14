#include <G4StateManager.hh>
#include <G4ApplicationState.hh>
//#include <G4Threading.hh>
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
  : G4MTRunManager(),
#else
  : G4RunManager(),
#endif
    fRunConfiguration(configuration)
{
/// Standard constructor
  SetVerboseLevel(2);

  // There might be some steps the user needs to do before everything is ready


#ifndef G4MULTITHREADED
  auto navigator = fRunConfiguration->CreateNavigatorForTracking();
  if(navigator) {
    G4TransportationManager::GetTransportationManager()->SetNavigatorForTracking(navigator);
    RegisterNavigator(navigator);
  }
#endif

#ifdef G4MULTITHREADED
  SetUserInitialization(fRunConfiguration->CreateWorkerInitialization());
#endif

}

void O2G4RunManager::RegisterNavigator(G4Navigator* userNavigator)
{
  fUserNavigators.emplace_back(userNavigator);
}

void O2G4RunManager::Initialize()
{
  #ifdef G4MULTITHREADED
    G4MTRunManager::Initialize();
  #else
    G4RunManager::Initialize();
  #endif

  //fRunConfiguration->Initialize();
}


#undef PARENT_CLASS
