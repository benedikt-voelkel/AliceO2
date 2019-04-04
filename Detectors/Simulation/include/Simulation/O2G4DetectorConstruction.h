#ifndef O2G4_DETECTOR_CONSTRUCTION_H
#define O2G4_DETECTOR_CONSTRUCTION_H

#include <G4VUserDetectorConstruction.hh>

class G4VPhysicalVolume;

class O2G4DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    O2G4DetectorConstruction();
    virtual ~O2G4DetectorConstruction() = default;
    /// Not implemented
    O2G4DetectorConstruction(const O2G4DetectorConstruction&) = delete;
    O2G4DetectorConstruction& operator=(const O2G4DetectorConstruction&) = delete;

    // methods
    G4VPhysicalVolume* Construct() override final;
    void ConstructSDandField() override final;

    //void SlaveO2G4DetectorConstruction();

  private:
    G4VPhysicalVolume* fWorld;
};

#endif // O2G4_DETECTOR_CONSTRUCTION_H
