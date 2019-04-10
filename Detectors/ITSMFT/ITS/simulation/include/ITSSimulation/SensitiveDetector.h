#ifndef ASDFG
#define ASDFG

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"


class SensitiveDetector : public G4VSensitiveDetector

  SensitiveDetector(o2::ITS::Detector* sd) : G4VSensitiveDetector("ITS"), mSensitiveDetector(sd)
  {}

  ~SensitiveDetector() = default;

  public:
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override
    {
      return true;
    }

  private:
    o2::ITS::Detector* mSensitiveDetector;

#endif
