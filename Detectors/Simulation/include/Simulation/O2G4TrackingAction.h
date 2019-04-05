#ifndef O2G4_TRACKING_ACTION_H
#define O2G4_TRACKING_ACTION_H

#include <G4UserTrackingAction.hh>

#include "O2SimInterface.h"

class G4Track;

class O2G4TrackingAction : public G4UserTrackingAction, public O2SimInterface
{
  public:
    O2G4TrackingAction();
    virtual ~O2G4TrackingAction() = default;

    /// Not implemented
    O2G4TrackingAction(const O2G4TrackingAction&) = delete;
    O2G4TrackingAction& operator=(const O2G4TrackingAction&) = delete;

    void PreUserTrackingAction(const G4Track* track) override final;

    void PostUserTrackingAction(const G4Track* track) override final;


    // methods
    EExitStatus Initialize() override final;
};

#endif // O2G4_TRACKING_ACTION_H
