#ifndef O2G4_PRIMARY_GENERATOR_ACTION_H
#define O2G4_PRIMARY_GENERATOR_ACTION_H

#include <G4VUserPrimaryGeneratorAction.hh>
#include <globals.hh>
#include <G4ThreeVector.hh>

#include "Generators/PrimaryGenerator.h"
#include "SimulationDataFormat/Stack.h"
#include "SimulationDataFormat/MCEventHeader.h"

#include "O2SimInterface.h"

class G4Event;
class G4PrimaryVertex;

class O2G4PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction, public O2SimInterface
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

    void SetPrimaryGenerator(FairPrimaryGenerator* primaryGenerator);

  private:
    void AttachPrimariesToEvent(G4Event* event);

  private:

    /// Option to skip particles which do not exist in Geant4
    G4bool  fSkipUnknownParticles;
    std::unique_ptr<o2::Data::Stack> fStack;
    std::unique_ptr<o2::eventgen::PrimaryGenerator> fPrimaryGenerator;
    std::unique_ptr<o2::dataformats::MCEventHeader> fEventHeader;
};

// inline functions

/// Set the option to skip particles which do not exist in Geant4
inline void O2G4PrimaryGeneratorAction::SetSkipUnknownParticles(G4bool value)
{ fSkipUnknownParticles = value; }

/// Return the option to skip particles which do not exist in Geant4
inline  G4bool O2G4PrimaryGeneratorAction::GetSkipUnknownParticles() const
{ return fSkipUnknownParticles; }

#endif // O2G4_PRIMARY_GENERATOR_ACTION_H
