// @(#)root/g4root:$Id$
// Author: Andrei Gheata   07/08/06

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/// \file O2G4DetectorConstruction.cxx
/// \brief Implementation of the O2G4DetectorConstruction class
///
/// \author A. Gheata; CERN

#include <utility> // std::pair

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#endif
#include "G4UnitsTable.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4FieldManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4GeometryManager.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"

#include "TList.h"
#include "TG4RootNavigator.h"
#include "TG4RootSolid.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TRefArray.h"
#include "TObjArray.h"
#include "TDatabasePDG.h"
#include "THashList.h"
#include "TIterator.h"
#include "TParticlePDG.h"


#include "FairVolumeList.h"

#include "TVirtualMCApplication.h"

#include "O2G4SensitiveDetector.h"
#include "O2G4DetectorConstruction.h"


//ClassImp(O2G4DetectorConstruction)

//______________________________________________________________________________
O2G4DetectorConstruction::O2G4DetectorConstruction()
                            : G4VUserDetectorConstruction(),
                              O2SimInterface(),
                              fIsConstructed(kFALSE),
                              fGeometry(nullptr),
                              fTopPV(nullptr)
{
/// Dummy ctor.
}

//______________________________________________________________________________
O2G4DetectorConstruction::O2G4DetectorConstruction(TGeoManager *geom)
                            : G4VUserDetectorConstruction(),
                              O2SimInterface(),
                              fIsConstructed(kFALSE),
                              fGeometry(geom),
                              fTopPV(nullptr)
{
/// Default ctor.
}

//______________________________________________________________________________
O2G4DetectorConstruction::~O2G4DetectorConstruction()
{
/// Destructor. Cleans all G4 geometry objects created.
//   if (fGeometry) delete fGeometry;
#ifdef G4GEOMETRY_VOXELDEBUG
   G4cout << "Deleting Materials ... ";
#endif
   G4MaterialTable *mtab = (G4MaterialTable*)G4Material::GetMaterialTable();
   std::vector<G4Material*>::iterator pos;
   G4int icount = 0;
   for (pos=mtab->begin(); pos!=mtab->end(); pos++) {
      if (*pos) {
         delete *pos;
         icount++;
      }
   }
#ifdef G4GEOMETRY_VOXELDEBUG
   G4cout << icount << " materials deleted !" << G4endl;
   G4cout << "Deleting Elements ... ";
#endif
   G4ElementTable *eltab = (G4ElementTable*)G4Element::GetElementTable();
   std::vector<G4Element*>::iterator pos1;
   icount = 0;
   for (pos1=eltab->begin(); pos1!=eltab->end(); pos1++) {
      if (*pos1) {
         delete *pos1;
         icount++;
      }
   }
#ifdef G4GEOMETRY_VOXELDEBUG
   G4cout << icount << " elements deleted !" << G4endl;
   G4cout << "Deleting Rotations ... ";
#endif
   G4PhysicalVolumeStore* pvstore = G4PhysicalVolumeStore::GetInstance();
   std::vector<G4VPhysicalVolume*>::iterator pos2;
   icount = 0;
   for (pos2=pvstore->begin(); pos2!=pvstore->end(); pos2++) {
      if (*pos2 && (*pos2)->GetRotation()) {
         delete (*pos2)->GetRotation();
         icount++;
      }
   }
#ifdef G4GEOMETRY_VOXELDEBUG
   G4cout << icount << " rotations deleted !" << G4endl;
#endif
   G4GeometryManager *mgr = G4GeometryManager::GetInstance();
   mgr->OpenGeometry();
   pvstore->Clean();
   G4LogicalVolumeStore *lvstore = G4LogicalVolumeStore::GetInstance();
   lvstore->Clean();
   G4SolidStore *sstore = G4SolidStore::GetInstance();
   sstore->Clean();
}

void O2G4DetectorConstruction::ExtractSensitiveVolumes(o2::base::Detector* detector)
{
  if(fSVMap.find(detector) != fSVMap.end()) {
    G4cout << "WARNING: The SVs of SD " << detector->GetName() << " have already been extracted" << G4endl;
    return;
  }
  if(detector->svList->GetEntries() == 0) {
    G4cout << "No SVs found for module " << detector->GetName() << G4endl;
    return;
  }

  for (G4int i = 0; i < detector->svList->GetEntries(); i++) {
    auto fairVol = static_cast<FairVolume*>(detector->svList->At(i));
    if(!fairVol) {
      G4cout << "######### Cannot retrieve FairVolume in list with index " << i << G4endl;
      continue;
    }
    auto tgeoVol = fGeometry->GetVolume(fairVol->GetName());
    if(!tgeoVol) {
      G4cout << "######### Cannot find TGeoVolume for FairVolumeName " << fairVol->GetName() << G4endl;
      continue;
    }
    auto g4vol = GetG4Volume(tgeoVol);
    if(!g4vol) {
      G4cout << "######### Cannot find G4LogicalVolume matching TGeoVolume " << tgeoVol->GetName() << G4endl;
      continue;
    }
    fSVMap[detector][g4vol] = tgeoVol;
  }
  G4cout << "######### Extracted SVs for detector " << detector->GetName() << G4endl;
}

void O2G4DetectorConstruction::AddModule(FairModule* module)
{
  auto detector = dynamic_cast<o2::base::Detector*>(module);
  if(detector) {
    fSDs.push_back(detector);
  } else {
    fPassive.push_back(module);
  }
  std::cerr << "######### Added module " << module->GetName() << std::endl;
  fModules.push_back(module);
}


//______________________________________________________________________________
G4VPhysicalVolume *O2G4DetectorConstruction::Construct()
{

  if(fModules.size() == 0) {
    G4Exception("O2G4DetectorConstruction::Construct",
                "G4Root_F001", FatalException,
                "No modules registered");
  }

  if (fTopPV) return fTopPV;


  // NOTE
  // For now adapt what is necessary from FairMCApplication::ConstructGeometry()


  G4int NoOfVolumes=0;
  G4int NoOfVolumesBefore=0;
  G4int ModId=0;

  TObjArray* tgeovolumelist = fGeometry->GetListOfVolumes();

  for(auto& mod : fModules) {
    G4cout << "Construct geometry for module " << mod->GetName() << G4endl;
    NoOfVolumesBefore=tgeovolumelist->GetEntriesFast();
    mod->InitParContainers();
    mod->ConstructGeometry();
    ModId=mod->GetModId();
    NoOfVolumes=tgeovolumelist->GetEntriesFast();
    for (G4int n = NoOfVolumesBefore; n < NoOfVolumes; n++) {
      TGeoVolume* v = (TGeoVolume*) tgeovolumelist->At(n);
      fModVolMap.insert(std::pair<G4int,G4int>(v->GetNumber(), ModId));
    }
  }

  // FIXME Also taken from FairMCApplication. For some reason TGeoManager needs some info about PDGs????
  G4int Counter=0;
  TDatabasePDG* pdgDatabase = TDatabasePDG::Instance();
  const THashList *list = pdgDatabase->ParticleList();
  if(list == 0) {
    pdgDatabase->ReadPDGTable();
  }
  list = pdgDatabase->ParticleList();
  if(list != 0) {
    TIterator *particleIter = list->MakeIterator();
    TParticlePDG *Particle = 0;
    while((Particle=dynamic_cast<TParticlePDG*> (particleIter->Next())) && (Counter <= 256)) {
      TString Name = fGeometry->GetPdgName(Particle->PdgCode());
      //    LOG(info) << Counter <<" : Particle name: "<< Name.Data() << " PDG " << Particle->PdgCode();
      if(Name == "XXX") {
        fGeometry->SetPdgName(Particle->PdgCode(), Particle->GetName());
      }
      Counter++;
    }
    delete particleIter;
  }

  for(auto& mod : fModules) {
    mod->ModifyGeometry();
  }

  fGeometry->RefreshPhysicalNodes(kFALSE);

  for(auto& det : fSDs) {
    // TODO Enable for physics cuts
    //det->SetSpecialPhysicsCuts();
    // Since it's sensitive, get TGeoVolumes to build SDs later
    G4cout << "Initialize detector " << det->GetName() << G4endl;
    if(det->IsActive()) {
      det->Initialize();
      det->Register();
    }
  }

  for(auto& mod : fPassive) {
    // Only special physics cuts
    //det->SetSpecialPhysicsCuts();
  }

  TGeoVolume *top = (TGeoVolume*)fGeometry->GetListOfVolumes()->First();
  fGeometry->SetTopVolume(top);
  G4int NoOfVolumesBeforeClose = tgeovolumelist->GetEntries();
  fGeometry->CloseGeometry();
  G4int NoOfVolumesAfterClose = tgeovolumelist->GetEntries();

  // Check if CloseGeometry has modified the volume list which might happen for
  // runtime shapes (parametrizations,etc). If this is the case, our lookup structures
  // built above are likely out of date. Issue at least a warning here.
  if (NoOfVolumesBeforeClose != NoOfVolumesAfterClose) {
    LOG(error) << "TGeoManager::CloseGeometry() modified the volume list from "
               << NoOfVolumesBeforeClose << " to " << NoOfVolumesAfterClose << "\n"
               << "This almost certainly means inconsistent lookup structures used in simulation/stepping.\n";
  }

  if (!fGeometry || !fGeometry->IsClosed()) {
    G4Exception("O2G4DetectorConstruction::Construct",
                "G4Root_F001", FatalException,
                "Cannot create O2G4DetectorConstruction without closed ROOT geometry !");
  }

  // Convert reflections via TGeo reflection factory
  fGeometry->ConvertReflections();
  CreateG4Materials();
  //CreateG4LogicalVolumes();
  CreateG4PhysicalVolumes();
  G4cout << "### INFO: O2G4DetectorConstruction::Construct() finished" << G4endl;

  for(auto& det : fSDs) {
    // TODO What exactly happens here?
    det->addAlignableVolumes();
    // Needs to be done here since SVs are defined during detector's initialization
    // and G4 geometry had to be constructed
    ExtractSensitiveVolumes(det);
  }

  G4cout << "gGeoManager at " << gGeoManager << "\n"
         << "fGeoManager at " << fGeometry << G4endl;

  fIsConstructed = kTRUE;

  // That is set automatically assuming the first node in the list is the right one
  // TODO Make this more robust
  return fTopPV;
}

//______________________________________________________________________________
void O2G4DetectorConstruction::ConstructSDandField()
{
  auto sdManager = G4SDManager::GetSDMpointer();
  O2G4SensitiveDetector* g4sd = nullptr;

  std::cout << "######### ConstructSDandField" << std::endl;

  for(auto& it : fSVMap) {
    std::cout << "######### Build SD for " << it.first->GetName() << std::endl;
    auto detClone = static_cast<o2::base::Detector*>(it.first->CloneModule());
    // Need to do this for now to keep track of these and ProcessHits is re-directed
    // to the correpsonding thread-local one
    fSDClones.push_back(detClone);
    g4sd = new O2G4SensitiveDetector(detClone);
    sdManager->AddNewDetector(g4sd);
    for(auto& volMap : it.second) {
      volMap.first->SetSensitiveDetector(g4sd);
      g4sd->MapG4LVToTGoTGeoVolumeID(volMap.first, volMap.second->GetNumber());
    }
  }
  sdManager->ListTree();
}

//______________________________________________________________________________
void O2G4DetectorConstruction::CreateG4LogicalVolumes()
{
/// Create logical volumes for GEANT4 based on TGeo volumes.
   TIter next(fGeometry->GetListOfVolumes());
   TGeoVolume *vol;
   while ((vol=(TGeoVolume*)next())) {
      CreateG4LogicalVolume(vol);
   }
   G4cout << "===> GEANT4 logical volumes created and mapped to TGeo ones..." << G4endl;
}

//______________________________________________________________________________
void O2G4DetectorConstruction::CreateG4PhysicalVolumes()
{
/// Create physical volumes for GEANT4 based on TGeo hierarchy.
   TGeoNode *node = fGeometry->GetTopNode();
   fTopPV = CreateG4PhysicalVolume(node);
   TGeoIterator next(fGeometry->GetTopVolume());
   TGeoNode *mother;
   while ((node=next())) {
      mother = next.GetNode(next.GetLevel()-1);
      if (mother && node->GetMotherVolume() != mother->GetVolume())
         node->SetMotherVolume(mother->GetVolume());
      CreateG4PhysicalVolume(node);
   }

   G4cout << "===> GEANT4 physical volumes created and mapped to TGeo hierarchy..." << G4endl;
}

//______________________________________________________________________________
void O2G4DetectorConstruction::CreateG4Materials()
{
/// Create GEANT4 native materials and map them to the corresponding TGeo ones.
   if (G4UnitDefinition::GetUnitsTable().size()==0) G4UnitDefinition::BuildUnitsTable();
//   G4cout << "Units table: " << G4endl;
//   G4UnitDefinition::PrintUnitsTable();
//   CreateG4Elements();
   TIter next(fGeometry->GetListOfMaterials());
   TGeoMaterial *mat;
   while ((mat=(TGeoMaterial*)next())) CreateG4Material(mat);
   G4cout << "===> GEANT4 materials created and mapped to TGeo ones..." << G4endl;
}

//______________________________________________________________________________
void O2G4DetectorConstruction::CreateG4Elements()
{
/// Create all necessary G4 elements.
   TGeoElementTable *table = fGeometry->GetElementTable();
   Int_t nelements = table->GetNelements();
   TGeoElement *elem;
   G4double a,z;
   G4String name, symbol;
   for (Int_t i=0; i<nelements; i++) {
      elem = table->GetElement(i);
      a = G4double(elem->A())*(g/mole);
      z = G4double(elem->Z());
      if ((z<1) || (z>101)) continue;
      name = elem->GetTitle();
      symbol = elem->GetName();
      new G4Element(name, symbol, z, a);
   }
   G4cout << "===> GEANT4 elements created..." << G4endl;
}

//______________________________________________________________________________
G4LogicalVolume *O2G4DetectorConstruction::CreateG4LogicalVolume(TGeoVolume *vol)
{
/// Create a G4LogicalVolume object based on a TGeo one. If already created
/// return just a pointer to the existing one.
   if (!vol) return NULL;
   G4LogicalVolume *pVolume = GetG4Volume(vol);
   if (pVolume) return pVolume;
   G4String sname(vol->GetName());
   G4VSolid *pSolid = CreateG4Solid(vol->GetShape());
   if (!pSolid) {
      G4ExceptionDescription description;
      description << "      "
        << "Cannot make solid from shape: " << vol->GetShape()->GetName();
      G4Exception("O2G4DetectorConstruction::CreateG4LogicalVolume",
                  "G4Root_F002", FatalException, description);
   }
   G4Material *pMaterial = 0;
   if (vol->IsAssembly()) {
      pMaterial = GetG4Material((TGeoMaterial*)fGeometry->GetListOfMaterials()->At(0));
   } else {
      pMaterial = GetG4Material(vol->GetMedium()->GetMaterial());
   }
   if (!pMaterial) {
      G4ExceptionDescription description;
      description << "      "
         << "Cannot make material for volume: " << vol->GetName() << G4endl;
      G4Exception("O2G4DetectorConstruction::CreateG4LogicalVolume",
                  "G4Root_F003", FatalException, description);
   }
   pVolume = new G4LogicalVolume(pSolid, pMaterial, sname,
                                                  NULL, NULL, NULL, false);
   fG4VolumeMap.insert(G4VolumeVal_t(vol, pVolume));
   fVolumeMap.insert(VolumeVal_t(pVolume, vol));
   return pVolume;
}

//______________________________________________________________________________
G4VPhysicalVolume *O2G4DetectorConstruction::CreateG4PhysicalVolume(TGeoNode *node)
{
/// Create a G4VPhysicalVolume object based on a TGeo node.
   if (!node) return NULL;
   node->cd();
   G4VPhysicalVolume *pPhysicalVolume = GetG4VPhysicalVolume(node);
   if (pPhysicalVolume) return pPhysicalVolume;
   TGeoMatrix *mat = node->GetMatrix();
   const Double_t *tr = mat->GetTranslation();
   G4ThreeVector tlate(tr[0]*cm, tr[1]*cm, tr[2]*cm);
   G4RotationMatrix *pRot = CreateG4Rotation(mat);
   G4String pName(node->GetVolume()->GetName());
   G4LogicalVolume *pCurrentLogical = CreateG4LogicalVolume(node->GetVolume());
   if (!pCurrentLogical) {
      G4ExceptionDescription description;
      description << "      "
         << "No G4 volume created for TGeo node " << node->GetName() << G4endl;
      G4Exception("O2G4DetectorConstruction::CreateG4PhysicalVolume",
                  "G4Root_F004", FatalException, description);
   }
   G4LogicalVolume *pMotherLogical = CreateG4LogicalVolume(node->GetMotherVolume());
   if (!pMotherLogical && node!=fGeometry->GetTopNode()) {
      G4ExceptionDescription description;
      description << "      "
         << "No G4 mother volume crated for TGeo node " << node->GetName();
      G4Exception("O2G4DetectorConstruction::CreateG4PhysicalVolume",
                  "G4Root_F005", FatalException, description);
   }
   G4bool pMany = false;
   G4int pCopyNo = node->GetNumber();

   pPhysicalVolume = new G4PVPlacement(pRot,tlate,pCurrentLogical,pName,
                                       pMotherLogical,pMany,pCopyNo);
   fG4PVolumeMap.insert(G4PVolumeVal_t(node, pPhysicalVolume));
   fPVolumeMap.insert(PVolumeVal_t(pPhysicalVolume, node));
   return pPhysicalVolume;
}

//______________________________________________________________________________
G4Material *O2G4DetectorConstruction::CreateG4Material(const TGeoMaterial *mat)
{
/// Create a GEANT4 material based on a TGeo one. If already created return
/// just a pointer to the existing one.
   G4Material *pMaterial = GetG4Material(mat);
   if (pMaterial) return pMaterial;
   G4State state = kStateUndefined;
   G4double temp = mat->GetTemperature();
   G4double pressure = mat->GetPressure();
   switch (mat->GetState()) {
      case TGeoMaterial::kMatStateUndefined :
         state = kStateUndefined;
         break;
      case TGeoMaterial::kMatStateSolid :
         state = kStateSolid;
         break;
      case TGeoMaterial::kMatStateLiquid :
         state = kStateLiquid;
         break;
      case TGeoMaterial::kMatStateGas :
         state = kStateGas;
         break;
   }
   G4String elname, symbol;
   TGeoElementTable *table = fGeometry->GetElementTable();
   G4String name(mat->GetName());
   G4double density = mat->GetDensity()*(g/cm3);
   if (density<universe_mean_density || mat->GetZ()<1.) {
      density = universe_mean_density;
      pMaterial = new G4Material(name, 1., 1.01*g/mole, density, kStateGas,
                                 STP_Temperature, 3.e-18*pascal);
      fG4MaterialMap.insert(G4MaterialVal_t(mat, pMaterial));
//      G4cout << pMaterial << G4endl;
      return pMaterial;
   }

   if (mat->IsMixture()) {
      // Mixtures
      const TGeoMixture *mixt = (const TGeoMixture *)mat;
      G4int nComponents = mixt->GetNelements();
//      G4cout << "Creating G4 mixture "<< name << G4endl;
      pMaterial = new G4Material(name, density, nComponents,state,temp,pressure);
      for (Int_t i=0; i<nComponents; i++) {
//         TGeoElement *elem = mixt->GetElement(i);
//         name = elem->GetTitle();
//         G4Element *pElement = G4Element::GetElement(name);
         TGeoElement *elem = table->GetElement(Int_t(mixt->GetZmixt()[i]));
         if (!elem) {
            G4ExceptionDescription description;
            description << "      "
              << "Woops: no element corresponding to Z=" << Int_t(mixt->GetZmixt()[i]);
            G4Exception("O2G4DetectorConstruction::CreateG4Material",
                        "G4Root_F006", FatalException, description);
         }
         elname = elem->GetTitle();
         symbol = elem->GetName();
         G4Element *pElement = new G4Element(elname, symbol, G4double(mixt->GetZmixt()[i]), G4double(mixt->GetAmixt()[i])*(g/mole));
         pMaterial->AddElement(pElement, mixt->GetWmixt()[i]);
      }
   } else {
      // Materials with 1 element.
//      G4cout << "Creating G4 material "<< name << G4endl;
      pMaterial = new G4Material(name, G4double(mat->GetZ()),
                                 mat->GetA()*g/mole, density, state, temp, pressure);
   }
   fG4MaterialMap.insert(G4MaterialVal_t(mat, pMaterial));
//   G4cout << pMaterial << G4endl;
   return pMaterial;
}

//______________________________________________________________________________
G4RotationMatrix *O2G4DetectorConstruction::CreateG4Rotation(const TGeoMatrix *matrix)
{
/// Create a G4Transform3D object based on a TGeo matrix. If already created
/// return just a pointer to the existing one.
   G4RotationMatrix *g4rot = 0;
   if (matrix->IsRotation()) {
//      matrix->Print();
      const Double_t *marray = matrix->GetRotationMatrix();
      Double_t invmat[9];
      invmat[0] = marray[0];
      invmat[1] = marray[3];
      invmat[2] = marray[6];
      invmat[3] = marray[1];
      invmat[4] = marray[4];
      invmat[5] = marray[7];
      invmat[6] = marray[2];
      invmat[7] = marray[5];
      invmat[8] = marray[8];
      CLHEP::HepRep3x3 mclhep(invmat);
      g4rot = new G4RotationMatrix(mclhep);
//      G4cout << *g4rot << G4endl;
   }
   return g4rot;
}

//______________________________________________________________________________
G4VSolid *O2G4DetectorConstruction::CreateG4Solid(TGeoShape *shape)
{
/// Create a G4 generic solid working with any TGeo shape. If already created
/// return just a pointer to the existing one.
   return new TG4RootSolid(shape);
   return NULL;
}

//______________________________________________________________________________
G4Material *O2G4DetectorConstruction::GetG4Material(const TGeoMaterial *mat) const
{
/// Retreive a G4 material mapped to a ROOT material.
   G4MaterialIt_t it = fG4MaterialMap.find(mat);
   if (it != fG4MaterialMap.end()) return it->second;
   return NULL;
}

//______________________________________________________________________________
G4LogicalVolume *O2G4DetectorConstruction::GetG4Volume(const TGeoVolume *vol) const
{
/// Retreive a G4 logical volume mapped to a ROOT volume.
   G4VolumeIt_t it = fG4VolumeMap.find(vol);
   if (it != fG4VolumeMap.end()) return it->second;
   return NULL;
}

//______________________________________________________________________________
TGeoVolume *O2G4DetectorConstruction::GetVolume(const G4LogicalVolume *g4vol) const
{
/// Retreive a TGeo logical volume mapped to a G4 volume.
   VolumeIt_t it = fVolumeMap.find(g4vol);
   if (it != fVolumeMap.end()) return it->second;
   return NULL;
}

//______________________________________________________________________________
G4VPhysicalVolume *O2G4DetectorConstruction::GetG4VPhysicalVolume(const TGeoNode *node) const
{
/// Retreive a G4 physical volume mapped to a ROOT node.
   G4PVolumeIt_t it = fG4PVolumeMap.find(node);
   if (it != fG4PVolumeMap.end()) return it->second;
   return NULL;
}

//______________________________________________________________________________
TGeoNode *O2G4DetectorConstruction::GetNode(const G4VPhysicalVolume *g4pvol) const
{
/// Retreive a TGeo node mapped to a G4 physical volume.
   PVolumeIt_t it = fPVolumeMap.find(g4pvol);
   if (it != fPVolumeMap.end()) return it->second;
   return NULL;
}
