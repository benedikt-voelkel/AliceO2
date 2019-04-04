#ifndef O2_SIM_INTERFACE_H
#define O2_SIM_INTERFACE_H

#include <G4VUserDetectorConstruction.hh>

class G4VPhysicalVolume;

enum class EExitStatus {kSUCCESS, kFAIL, kUNKNOWN};

class O2SimInterface
{
  public:
    O2SimInterface() = default;
    virtual ~O2SimInterface() = default;
    /// Not implemented
    O2G4DetectorConstruction(const O2G4DetectorConstruction&) = delete;
    O2G4DetectorConstruction& operator=(const O2G4DetectorConstruction&) = delete;

    // methods
    virtual EExitStatus Initialize()
    {
      fIsInitialized = true;
      return EExitStatus::kSUCCESS;
    }
    virtual EExitStatus Finalize()
    {
      fIsFinalized = true;
      return EExitStatus::kSUCCESS;
    }

    //void SlaveO2G4DetectorConstruction();

  private:
    EExitStatus fLatestStatus = kUNKNOWN;
    G4bool fIsInitialized = false;
    G4bool fIsFinalized = false;
};

#endif // O2_SIM_INTERFACE_H
