#ifndef O2_EVENT_ACTION_H
#define O2_EVENT_ACTION_H

#include <G4UserEventAction.hh>
#include <globals.hh>

class G4Event;

/// \ingroup event
/// \brief Actions at the beginning and the end of event.
///
/// \author I. Hrivnacova; IPN, Orsay

class O2G4EventAction : public G4UserO2G4EventAction, public O2SimInterface
{
  public:
    O2G4EventAction();
    virtual ~O2G4EventAction() = default;

    /// Not implemented
    O2G4EventAction(const O2G4EventAction&) = delete;
    /// Not implemented
    O2G4EventAction& operator=(const O2G4EventAction&) = delete;

    // methods
    //void LateInitialize();
    void BeginOfO2G4EventAction(const G4Event* event) override final;
    void EndOfO2G4EventAction(const G4Event* event) override final;

    void Initialize() override final;

    // set methods
    //void SetMCStack(TVirtualMCStack*  mcStack);
    //void SetPrintMemory(G4bool printMemory);
    //void SetSaveRandomStatus(G4bool saveRandomStatus);

    // get methods
    //G4bool  GetPrintMemory() const;
    //G4bool  GetSaveRandomStatus() const;

  private:


    // data members
    //O2G4EventActionMessenger   fMessenger; ///< messenger

    /// Cached pointer to thread-local tracking action
    TrackingAction*  fTrackingAction;

    /// Control for printing memory usage
    //G4bool  fPrintMemory;

    /// Control for saving random engine status for each event
    //G4bool  fSaveRandomStatus;
};

// inline methods
/*
inline void O2G4EventAction::SetPrintMemory(G4bool printMemory) {
  /// Set option for printing memory usage
  fPrintMemory = printMemory;
}

inline G4bool O2G4EventAction::GetPrintMemory() const {
  /// Return the option for printing memory usage
  return fPrintMemory;
}
*/
#endif // EVENT_ACTION_H
