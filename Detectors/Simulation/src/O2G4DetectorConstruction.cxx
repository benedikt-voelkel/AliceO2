//------------------------------------------------
// The Geant4 Virtual Monte Carlo package
// Copyright (C) 2007 - 2014 Ivana Hrivnacova
// All rights reserved.
//
// For the licensing terms see geant4_vmc/LICENSE.
// Contact: root-vmc@cern.ch
//-------------------------------------------------

/// \file O2G4DetectorConstruction.cxx
/// \brief Implementation of the O2G4DetectorConstruction class
///
/// \author I. Hrivnacova; IPN, Orsay

#include "O2G4DetectorConstruction.h"
#include "TG4GeometryManager.h"
#include "TG4GeometryServices.h"
#include "TG4Globals.h"

#include <G4VPhysicalVolume.hh>

//_____________________________________________________________________________
O2G4DetectorConstruction::O2G4DetectorConstruction()
  : G4VUserDetectorConstruction(),
    fWorld(nullptr)
{
}

//
// public methods
//

//_____________________________________________________________________________
G4VPhysicalVolume* O2G4DetectorConstruction::Construct()
{
/// Construct geometry is delegated to TG4GeometryManager
// --

  // Build G4 geometry
  TG4GeometryManager::Instance()->ConstructGeometry();

  fWorld = TG4GeometryServices::Instance()->GetWorld();

  return fWorld;
}

//_____________________________________________________________________________
void O2G4DetectorConstruction::ConstructSDandField()
{
/// Construct SDandField is delegated to TG4GeometryManager
// --

  TG4GeometryManager::Instance()->ConstructSDandField();
}
