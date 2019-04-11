#ifndef O2G4_SENSITIVE_DETECTOR_H
#define O2G4_SENSITIVE_DETECTOR_H

#include <memory>
#include <map>

#include <G4VSensitiveDetector.hh>

#include "G4LogicalVolume.hh"

#include "FairVolume.h"

class FairDetector;
class G4Step;
class G4TouchableHistory;

namespace o2
{
namespace base
{
class Detector;
}
}

class O2G4SensitiveDetector : public G4VSensitiveDetector
{
  public:
    O2G4SensitiveDetector() = delete;
    O2G4SensitiveDetector(o2::base::Detector* detector);
    ~O2G4SensitiveDetector() override = default;

    /// Not implemented
    //O2G4SensitiveDetector(const O2G4SensitiveDetector&) = delete;
    //O2G4SensitiveDetector& operator=(const O2G4SensitiveDetector&) = delete;

    //
    // methods
    //
    void MapG4LVToTGoTGeoVolumeID(G4LogicalVolume* g4lv, G4int tgeoVolID);


    //
    // inherited
    //
  protected:
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;


  private:
    o2::base::Detector* fO2Detector;
    /// Lookup TGeoVolume from G4LogicalVolume
    std::map<G4LogicalVolume*, G4int> fG4LVTGeoVolumeIDMap;

};


#endif // O2G4_SENSITIVE_DETECTOR_H
