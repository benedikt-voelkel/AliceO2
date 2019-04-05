#include <G4Event.hh>
#include <G4ParticleTable.hh>
#include <G4IonTable.hh>
#include <G4ParticleDefinition.hh>

// Moved after Root includes to avoid shadowed variables
// generated from short units names
#include <G4SystemOfUnits.hh>

#include "O2G4PrimaryGeneratorAction.h"


//_____________________________________________________________________________
O2G4PrimaryGeneratorAction::O2G4PrimaryGeneratorAction()
  : G4VUserPrimaryGeneratorAction(),
    O2SimInterface(),
    fSkipUnknownParticles(false)
{
}

//
// public methods
//

//_____________________________________________________________________________
void O2G4PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
/// Generate primary particles by the selected generator.
  for (auto& vertex : fVertices) {
    event->AddPrimaryVertex(vertex);
  }
}

void O2G4PrimaryGeneratorAction::ResetUserParticles()
{
  fVertices.clear();
}

void O2G4PrimaryGeneratorAction::AddUserParticle(G4int pdgCode,
                                             G4double time,
                                             const G4ThreeVector& position,
                                             G4double energy,
                                             const G4ThreeVector& momentum,
                                             const G4ThreeVector& polarization,
                                             G4double weight)
{
  G4ParticleDefinition* particleDefinition = nullptr;
  if (pdgCode != 0) {
    particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode);
  }

  if (!particleDefinition) {
    G4cout << "Cannot find particle with PDG code " << pdgCode << G4endl;
    if(!fSkipUnknownParticles) {
      G4Exception("O2G4PrimaryGeneratorAction::AddUserParticle", "Run0031",
                  FatalException, "Cannot find particle PDG code.");
    }
    return;
  }

  auto currentVertex = fVertices.back();
  auto previousPosition = currentVertex->GetPosition();
  auto previousTime = currentVertex->GetT0();

  if ( position != previousPosition || time != previousTime ) {
    // Create a new vertex if needed
    // (vertex objects are destroyed in G4EventManager::ProcessOneEvent()
    // when event is deleted)
    currentVertex = new G4PrimaryVertex(position, time);
    fVertices.push_back(currentVertex);
  }

  // Create the primary particle from the given info
  G4PrimaryParticle* primaryParticle = new G4PrimaryParticle(particleDefinition,
                                                             momentum.x(),
                                                             momentum.y(),
                                                             momentum.z(),
                                                             energy);

  // Set charge
  // TODO Do we need that?
  /*
  G4double charge = particleDefinition->GetPDGCharge();
  if ( G4IonTable::IsIon(particleDefinition) &&
       particleDefinition->GetParticleName() != "proton" ) {
    // Get dynamic charge defined by user
    TG4UserIon* userIon = particlesManager->GetUserIon(particle->GetName(), false);
    if ( userIon ) charge = userIon->GetQ() * eplus;
  }
  primaryParticle->SetCharge(charge);
  */

  // Set polarization
  primaryParticle->SetPolarization(polarization.x(), polarization.y(), polarization.z());

  // Set weight
  primaryParticle->SetWeight(weight);

  // TODO Check last pushed vertex position
  currentVertex->SetPrimary(primaryParticle);
}
