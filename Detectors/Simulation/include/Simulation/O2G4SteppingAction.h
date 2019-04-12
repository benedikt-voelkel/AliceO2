#ifndef O2G4_STEPPING_ACTION_H
#define O2G4_STEPPING_ACTION_H
//#include "TG4GeoTrackManager.h"

#include <globals.hh>
#include <G4UserSteppingAction.hh>

#include "O2SimInterface.h"

/*
class TG4SpecialControlsV2;
class TG4TrackManager;
class TG4StepManager;
class TG4StackPopper;
*/

class G4Track;
class G4Step;

class O2G4SteppingAction : public G4UserSteppingAction, public O2SimInterface
{
  enum {
    kMaxNofSteps = 30000,
    kMaxNofLoopSteps = 5
  };

  public:
    O2G4SteppingAction();
    virtual ~O2G4SteppingAction() = default;

    /// Not implemented
    O2G4SteppingAction(const O2G4SteppingAction&) = delete;
    /// Not implemented
    O2G4SteppingAction& operator=(const O2G4SteppingAction&) = delete;

    // methods
    EExitStatus Initialize() override final;
    void UserSteppingAction(const G4Step* step) override final;

    // set methods
    /*
    void SetLoopVerboseLevel(G4int level);
    void SetMaxNofSteps(G4int number);
    void SetSpecialControls(TG4SpecialControlsV2* specialControls);
    void SetIsPairCut(G4bool isPairCut);
    void SetCollectTracks(G4bool collectTracks);

    // get methods
    G4int GetLoopVerboseLevel() const;
    G4int GetMaxNofSteps() const;
    G4bool GetIsPairCut() const;
    G4bool GetCollectTracks() const;
    */

  protected:
    // methods
    void PrintTrackInfo(const G4Track* track) const;

  private:

    // static data members
    //static G4ThreadLocal O2G4SteppingAction*   fgInstance; ///< this instance
    //
    // methods

    void ProcessTrackIfLooping(const G4Step* step);
    /*
    void ProcessTrackIfOutOfRegion(const G4Step* step);
    void ProcessTrackIfBelowCut(const G4Step* step);
    void ProcessTrackOnBoundary(const G4Step* step);
    */
  private:
    G4int fLoopStepCounter;
    G4int fMaxNofSteps;
    G4int fNSteps;

};

#endif // O2G4_STEPPING_ACTION_H
