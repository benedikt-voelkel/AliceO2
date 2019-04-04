#ifndef O2G4_TRACKING_ACTION_H
#define O2G4_TRACKING_ACTION_H

#include <G4UserTrackingAction.hh>

class G4Track;

class O2G4TrackingAction : public G4UserTrackingAction, public O2SimInterface
{
  public:
    virtual ~O2G4TrackingAction() = default;

    /// Not implemented
    O2G4TrackingAction(const O2G4TrackingAction&) = delete;
    O2G4TrackingAction& operator=(const O2G4TrackingAction&) = delete;

    // methods
    void Initialize() override final;
};

#endif // O2G4_TRACKING_ACTION_H
