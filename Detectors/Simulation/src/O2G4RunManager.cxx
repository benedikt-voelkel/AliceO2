//------------------------------------------------
// The Geant4 Virtual Monte Carlo package
// Copyright (C) 2007, 2014 Ivana Hrivnacova
// All rights reserved.
//
// For the licensing terms see geant4_vmc/LICENSE.
// Contact: root-vmc@cern.ch
//-------------------------------------------------

/// \file O2G4RunManager.cxx
/// \brief Implementation of the O2G4RunManager class
///
/// \author I. Hrivnacova; IPN, Orsay
#define PARENT_CLASS G4RunManagerMT
#ifndef G4MULTITHREADED
#define PARENT_CLASS G4RunManager
#endif
#include "O2G4RunManager.h"
#include "O2SimInterface.h"

#include <G4StateManager.hh>
#include <G4ApplicationState.hh>
#include <G4Threading.hh>

#ifdef G4VIS_USE
#include <G4VisExecutive.hh>
#endif


//_____________________________________________________________________________
O2G4RunManager* O2G4RunManager::fgMasterInstance = 0;


//_____________________________________________________________________________
O2G4RunManager::O2G4RunManager(VO2G4RunConfiguration* configuration)
  : PARENT_CLASS(), O2SimInterface(),
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

EExitStatus O2G4RunManager::Initialize()
{
  return O2SimInterface::Initialize();
}


void O2G4RunManager::RegisterNavigator(G4Navigator* userNavigator)
{
  fUserNavigators.emplace_back(userNavigator);
}
