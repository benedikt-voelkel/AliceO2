
#include "DetectorsBase/Detector.h"

#include <G4Step.hh>
#include <G4TouchableHistory.hh>
#include <G4HCofThisEvent.hh>
#include <G4SDManager.hh>

#include "O2G4SensitiveDetector.h"

//_____________________________________________________________________________
O2G4SensitiveDetector::O2G4SensitiveDetector(o2::base::Detector* detector)
  : G4VSensitiveDetector(detector->GetName()), fO2Detector(detector),
    fHitsCollection(nullptr), fHCID(-1)
{
  G4String collName = SensitiveDetectorName + "_collection";
  collectionName.insert( "hodoscopeColl");
}



void O2G4SensitiveDetector::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection = new G4THitsCollection<G4VHit>(SensitiveDetectorName, collectionName[0]);
  if (fHCID<0) {
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
  }
  hce->AddHitsCollection(fHCID,fHitsCollection);
}


G4bool O2G4SensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist)
{
  G4cout << "######################## HIT in ITS O2G4SensitiveDetector ###################" << G4endl;
  G4cout << "Size of map is " << fG4LVTGeoVolumeIDMap.size() << G4endl;
  auto pv = aStep->GetPreStepPoint()->GetPhysicalVolume();
  auto it = fG4LVTGeoVolumeIDMap.find(pv->GetLogicalVolume());
  // First extract the TGeoVolume pointer
  if(it == fG4LVTGeoVolumeIDMap.end()) {
    return false;
  }
  // Turn into FairVolume pointer simply by comparing names
  return fO2Detector->ProcessHits(aStep, it->second);
}


void O2G4SensitiveDetector::MapG4LVToTGoTGeoVolumeID(G4LogicalVolume* g4lv, G4int tgeoVolID)
{
  if(fG4LVTGeoVolumeIDMap.find(g4lv) != fG4LVTGeoVolumeIDMap.end()) {
    G4Exception("O2G4SensitiveDetector::AddToG4LVTGeoNodeMap",
                "Run0012", FatalException,
                "Logical volume already registered");
  }
  fG4LVTGeoVolumeIDMap[g4lv] = tgeoVolID;
}
