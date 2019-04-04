#ifndef O2G4_RUN_ACTION_H
#define O2G4_RUN_ACTION_H

//------------------------------------------------
// The Geant4 Virtual Monte Carlo package
// Copyright (C) 2007 - 2014 Ivana Hrivnacova
// All rights reserved.
//
// For the licensing terms see geant4_vmc/LICENSE.
// Contact: root-vmc@cern.ch
//-------------------------------------------------

/// \file O2G4RunAction.h
/// \brief Definition of the O2G4RunAction class
///
/// \author I. Hrivnacova; IPN Orsay

#include <memory>

#include <G4UserRunAction.hh>
#include <globals.hh>

class G4Timer;
    // in order to avoid the odd dependency for the
    // times system function this declaration must be the first
class G4Run;


/// \ingroup run
/// \brief Actions at the beginning and the end of run.
///
/// \author I. Hrivnacova; IPN, Orsay

class O2G4RunAction : public G4UserO2G4RunAction, public O2SimInterface
{
  public:
    O2G4RunAction();
    virtual ~O2G4RunAction() = default;
    /// Not implemented
    O2G4RunAction(const O2G4RunAction&) = delete;
    O2G4RunAction& operator=(const O2G4RunAction&) = delete;

    // methods
    void BeginOfO2G4RunAction(const G4Run* run) override final;
    void EndOfO2G4RunAction(const G4Run* run) override final;

    void Initialize() override final;

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
