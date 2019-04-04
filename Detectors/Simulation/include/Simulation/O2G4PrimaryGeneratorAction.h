#ifndef O2G4_PRIMARY_GENERATOR_ACTION_H
#define O2G4_PRIMARY_GENERATOR_ACTION_H

#include "TG4Verbose.h"

#include <G4VUserPrimaryGeneratorAction.hh>
#include <globals.hh>

class TG4PrimaryGeneratorMessenger;

class G4Event;

/// \ingroup run
/// \brief Primary generator action defined via TVirtualMCStack
/// and TVirtualMCApplication.
///
/// \author I. Hrivnacova; IPN, Orsay

class O2G4PrimaryGeneratorAction : public G4VUserO2G4PrimaryGeneratorAction,
                                   public O2SimInterface
{
  public:
    O2G4PrimaryGeneratorAction();
    virtual ~O2G4PrimaryGeneratorAction() = default;

    // methods
    void GeneratePrimaries(G4Event* event) override final;
    void ResetUserParticles();
    void AddUserParticle(G4int pdgCode, G4double time, const G4ThreeVector& position,
                         G4double energy, const G4ThreeVector& momentum,
                         const G4ThreeVector& polarization, G4double weight);

    // set methods
    void SetSkipUnknownParticles(G4bool value);

    // get methods
    G4bool GetSkipUnknownParticles() const;

  private:

    /// Option to skip particles which do not exist in Geant4
    G4bool  fSkipUnknownParticles;
};

// inline functions

/// Set the option to skip particles which do not exist in Geant4
inline void O2G4PrimaryGeneratorAction::SetSkipUnknownParticles(G4bool value)
{ fSkipUnknownParticles = value; }

/// Return the option to skip particles which do not exist in Geant4
inline  G4bool O2G4PrimaryGeneratorAction::GetSkipUnknownParticles() const
{ return fSkipUnknownParticles; }

#endif // PRIMARY_GENERATOR_ACTION_H
