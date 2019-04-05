#include <G4VPhysicalVolume.hh>

#include "TG4GeometryManager.h"
#include "TG4GeometryServices.h"
#include "TG4Globals.h"

#include "O2G4DetectorConstruction.h"

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
