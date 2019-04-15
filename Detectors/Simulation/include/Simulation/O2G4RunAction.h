#ifndef O2G4_RUN_ACTION_H
#define O2G4_RUN_ACTION_H

#include <memory>

#include <G4UserRunAction.hh>
#include <globals.hh>

#include "O2SimInterface.h"

class G4Timer;
    // in order to avoid the odd dependency for the
    // times system function this declaration must be the first
class G4Run;


/// \ingroup run
/// \brief Actions at the beginning and the end of run.
///
/// \author I. Hrivnacova; IPN, Orsay

class O2G4RunAction : public G4UserRunAction, public O2SimInterface
{
  public:
    O2G4RunAction();
    virtual ~O2G4RunAction() = default;
    /// Not implemented
    O2G4RunAction(const O2G4RunAction&) = delete;
    O2G4RunAction& operator=(const O2G4RunAction&) = delete;

    // methods
    void BeginOfRunAction(const G4Run* run) override final;
    void EndOfRunAction(const G4Run* run) override final;

    // set methods
    //void SetSaveRandomStatus(G4bool saveRandomStatus);
    //void SetReadRandomStatus(G4bool readRandomStatus);
    //void SetRandomStatusFile(G4String RandomStatusFile);

  private:


    // static data members
    /// default name of the random engine status file to be read in
    //static const G4String  fgkDefaultRandomStatusFile;

     // data members
    //O2G4RunActionMessenger   fMessenger; ///< messenger
    //TG4CrossSectionManager  fCrossSectionManager; ///< cross section manager
    std::unique_ptr<G4Timer> fTimer; ///< G4Timer
    G4int fRunID; ///< run ID
};

#endif // O2G4_RUN_ACTION_H
