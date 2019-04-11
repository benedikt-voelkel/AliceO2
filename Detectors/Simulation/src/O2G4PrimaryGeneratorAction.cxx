#include <G4Event.hh>
#include <G4ParticleTable.hh>
#include <G4IonTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>

// Moved after Root includes to avoid shadowed variables
// generated from short units names
#include <G4SystemOfUnits.hh>

#include "TG4G3Units.h"

#include "Generators/GeneratorFactory.h"
#include "SimConfig/SimConfig.h"

#include "O2G4PrimaryGeneratorAction.h"


//_____________________________________________________________________________
O2G4PrimaryGeneratorAction::O2G4PrimaryGeneratorAction()
  : G4VUserPrimaryGeneratorAction(),
    O2SimInterface(),
    fSkipUnknownParticles(false),
    fStack(new o2::Data::Stack()),
    fPrimaryGenerator(new o2::eventgen::PrimaryGenerator()),
    fEventHeader(new o2::dataformats::MCEventHeader())
{
  o2::eventgen::GeneratorFactory::setPrimaryGenerator(o2::conf::SimConfig::Instance(), fPrimaryGenerator.get());
  fPrimaryGenerator->SetEvent(fEventHeader.get());
  std::cerr << "Constructed O2G4PrimaryGeneratorAction" << std::endl;
}

//
// public methods
//



//_____________________________________________________________________________
void O2G4PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  fStack->Reset();
  fPrimaryGenerator->GenerateEvent(fStack.get());
  AttachPrimariesToEvent(event);
}


void O2G4PrimaryGeneratorAction::AttachPrimariesToEvent(G4Event* event)
{

  auto particleTable = G4ParticleTable::GetParticleTable();
  G4PrimaryVertex* previousVertex = nullptr;
  G4ThreeVector previousPosition = G4ThreeVector();
  G4double previousTime = 0.;

  TParticle* particle = nullptr;
  int trackId = -1;
  while((particle = fStack->PopNextTrack(trackId))) {

    auto pdgCode = particle->GetPdgCode();
    if (pdgCode == 0) {
      G4Exception("O2G4PrimaryGeneratorAction::AttachPrimariesToEvent",
                  "Run0012", FatalException,
                  "PDG of requested particle is 0");
    }

    G4ParticleDefinition* particleDefinition = particleTable->FindParticle(pdgCode);
    if (!particleDefinition) {
      //TString text = "G4ParticleTable::FindParticle() failed for ";
      //text += TString(particle->GetName());
      //text += "  pdgEncoding=";
      //text += particle->GetPdgCode();
      //text += ".";
      if ( fSkipUnknownParticles ) {
        continue;
      }
      else {
        G4Exception("O2G4PrimaryGeneratorAction::AttachPrimariesToEvent",
                    "Run0012", FatalException,
                    "Unknown particle definition");
      }
    }

    // Get/Create vertex
    G4ThreeVector position(particle->Vx()*TG4G3Units::Length(),
                           particle->Vy()*TG4G3Units::Length(),
                           particle->Vz()*TG4G3Units::Length());
    G4double time = particle->T()*TG4G3Units::Time();
    G4PrimaryVertex* vertex;
    if ( previousVertex ==0 || position != previousPosition || time != previousTime ) {
      // Create a new vertex
      // in case position and time of gun particle are different from
      // previous values
      // (vertex objects are destroyed in G4EventManager::ProcessOneEvent()
      // when event is deleted)
      vertex = new G4PrimaryVertex(position, time);
      event->AddPrimaryVertex(vertex);

      previousVertex = vertex;
      previousPosition = position;
      previousTime = time;
    }
    else
      vertex = previousVertex;

    // Create a primary particle and add it to the vertex
    // (primaryParticle objects are destroyed in G4EventManager::ProcessOneEvent()
    // when event and then vertex is deleted)
    G4ThreeVector momentum(particle->Px()*TG4G3Units::Energy(),
                           particle->Py()*TG4G3Units::Energy(),
                           particle->Pz()*TG4G3Units::Energy());
    G4double energy = particle->Energy()*TG4G3Units::Energy();
    G4PrimaryParticle* primaryParticle
      = new G4PrimaryParticle(particleDefinition,
                              momentum.x(), momentum.y(), momentum.z(), energy);

    // Set charge
    G4double charge = particleDefinition->GetPDGCharge();
    /*
    if ( G4IonTable::IsIon(particleDefinition) &&
         particleDefinition->GetParticleName() != "proton" ) {
      // Get dynamic charge defined by user
      TG4UserIon* userIon = particlesManager->GetUserIon(particle->GetName(), false);
      if ( userIon ) charge = userIon->GetQ() * eplus;
    }
    */
    primaryParticle->SetCharge(charge);

    // Set polarization
    TVector3 polarization;
    particle->GetPolarisation(polarization);
    primaryParticle
      ->SetPolarization(polarization.X(), polarization.Y(), polarization.Z());

    // Set weight
    G4double weight =  particle->GetWeight();
    primaryParticle->SetWeight(weight);

    // Add primary particle to the vertex
    vertex->SetPrimary(primaryParticle);

  }
}
