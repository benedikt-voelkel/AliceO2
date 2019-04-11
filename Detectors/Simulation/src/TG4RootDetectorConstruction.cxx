// @(#)root/g4root:$Id$
// Author: Andrei Gheata   07/08/06

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/// \file TG4RootDetectorConstruction.cxx
/// \brief Implementation of the TG4RootDetectorConstruction class
///
/// \author A. Gheata; CERN

#include "TGeoManager.h"
#include "TGeoMatrix.h"

#include "TRefArray.h"

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

#include "TG4RootNavMgr.h"
#include "TList.h"
#include "TG4RootNavigator.h"
#include "TG4RootSolid.h"
#include "TG4RootDetectorConstruction.h"

#include "FairVolumeList.h"

#include "TVirtualMCApplication.h"

#include "O2G4SensitiveDetector.h"

//ClassImp(TG4RootDetectorConstruction)

//______________________________________________________________________________
TG4RootDetectorConstruction::TG4RootDetectorConstruction()
                            :G4VUserDetectorConstruction(),
                             fIsConstructed(kFALSE),
                             fGeometry(0),
                             fTopPV(0),
                             fSDInit(0)
{
/// Dummy ctor.
}

//______________________________________________________________________________
TG4RootDetectorConstruction::TG4RootDetectorConstruction(TGeoManager *geom)
                            :G4VUserDetectorConstruction(),
                             fIsConstructed(kFALSE),
                             fGeometry(geom),
                             fTopPV(0),
                             fSDInit(0)
{
/// Default ctor.
  TVirtualMCApplication::Instance()->ConstructGeometry();
  TGeoVolume *top = (TGeoVolume*)fGeometry->GetListOfVolumes()->First();
  fGeometry->SetTopVolume(top);
  fGeometry->CloseGeometry();
  Construct();
}

//______________________________________________________________________________
TG4RootDetectorConstruction::~TG4RootDetectorConstruction()
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
   if (fSDInit) delete fSDInit;
}

//______________________________________________________________________________
void TG4RootDetectorConstruction::Initialize(TVirtualUserPostDetConstruction *sdinit)
{
/// Main construct method.
   if (sdinit) {
      if (fSDInit) delete fSDInit;
      fSDInit = sdinit;
   }
   if (!fIsConstructed) {
      Construct();
      if (fSDInit) fSDInit->Initialize(this);
   }
}

//______________________________________________________________________________
G4VPhysicalVolume *TG4RootDetectorConstruction::Construct()
{
/// Main construct method.
   // First construct via TVirtualMCApplication
   // Now this assumes that the geometry is held by ROOT's TGeoManager
   if (!fGeometry || !fGeometry->IsClosed()) {
      G4Exception("TG4RootDetectorConstruction::Construct",
                  "G4Root_F001", FatalException,
                  "Cannot create TG4RootDetectorConstruction without closed ROOT geometry !");
   }
   if (fTopPV) return fTopPV;
   // Convert reflections via TGeo reflection factory
   fGeometry->ConvertReflections();
   CreateG4Materials();
//   CreateG4LogicalVolumes();
   CreateG4PhysicalVolumes();
   //TG4RootNavMgr *navMgr = TG4RootNavMgr::GetInstance(fGeometry);
   //TG4RootNavigator *nav = navMgr->GetNavigator();
   //nav->SetDetectorConstruction(this);
   //nav->SetWorldVolume(fTopPV);
   G4cout << "### INFO: TG4RootDetectorConstruction::Construct() finished" << G4endl;
   fIsConstructed = kTRUE;
   return fTopPV;
}

//______________________________________________________________________________
void TG4RootDetectorConstruction::ConstructSDandField()
{
  auto sdManager = G4SDManager::GetSDMpointer();

  O2G4SensitiveDetector* g4sd = nullptr;

  G4cout << "######### ConstructSDandField" << G4endl;


  for(auto& sd : fPotentialSDs) {
    if(!sd) {
      G4cout << "######### No potential SD" << G4endl;
      return;
    }
    if(!sd->svList) {
      G4cout << "######### No svList ind potential SD" << G4endl;
      continue;
    }
    if(sd->svList->GetEntries() == 0) {
      G4cout << "######### There are no SVs in potential SD " << sd->GetName() << G4endl;
    } else {
      g4sd = new O2G4SensitiveDetector(static_cast<o2::base::Detector*>(sd->CloneModule()));
      sdManager->AddNewDetector(g4sd);
      for (G4int i = 0; i < sd->svList->GetEntries(); i++) {
        auto fairVol = static_cast<FairVolume*>(sd->svList->At(i));
        if(!fairVol) {
          G4cout << "S######### Cannot retrieve FairVolume in list with index " << i << G4endl;
          continue;
        }
        auto tgeoVol = gGeoManager->GetVolume(fairVol->GetName());
        if(!tgeoVol) {
          G4cout << "S######### Cannot find TGeoVolume for FairVolumeName " << fairVol->GetName() << G4endl;
          continue;
        }
        auto g4vol = GetG4Volume(tgeoVol);
        if(!g4vol) {
          G4cout << "S######### Cannot find G4LogicalVolume matching TGeoVolume " << tgeoVol->GetName() << G4endl;
          continue;
        }
        G4cout << "S######### Set SD " << sd->GetName() << " to volume " << tgeoVol->GetName() << G4endl;
        g4vol->SetSensitiveDetector(g4sd);
        g4sd->MapG4LVToTGoTGeoVolumeID(g4vol, fairVol->getMCid());
      }
      G4cout << "######### Created SD " << sd->GetName() << G4endl;
      continue;
    }

    if(sd->vList->getEntries() == 0) {
      G4cout << "######### There are no Vs in potential SD " << sd->GetName() << G4endl;
      continue;
    }

    g4sd = new O2G4SensitiveDetector(static_cast<o2::base::Detector*>(sd->CloneModule()));
    sdManager->AddNewDetector(g4sd);
    for (G4int i = 0; i < sd->vList->getEntries(); i++) {
      auto fairVol = static_cast<FairVolume*>(sd->vList->At(i));
      if(!fairVol) {
        G4cout << "S######### Cannot retrieve FairVolume in list with index " << i << G4endl;
        continue;
      }
      auto tgeoVol = gGeoManager->GetVolume(fairVol->GetName());
      if(!tgeoVol) {
        G4cout << "S######### Cannot find TGeoVolume for FairVolumeName " << fairVol->GetName() << G4endl;
        continue;
      }
      auto g4vol = GetG4Volume(tgeoVol);
      if(!g4vol) {
        G4cout << "S######### Cannot find G4LogicalVolume matching TGeoVolume " << tgeoVol->GetName() << G4endl;
        continue;
      }
      G4cout << "S######### Set SD " << sd->GetName() << " to volume " << tgeoVol->GetName() << G4endl;
      g4vol->SetSensitiveDetector(g4sd);
      g4sd->MapG4LVToTGoTGeoVolumeID(g4vol, fairVol->getMCid());
    }
    G4cout << "######### Created SD " << sd->GetName() << G4endl;
  }
  /*
  G4cout << "TG4RootDetectorConstruction::ConstructSDandField" << G4endl;
  if (fSDInit) fSDInit->InitializeSDandField();
  G4cout << "### INFO: TG4RootDetectorConstruction::ConstructSDandField finished" << G4endl;
  */
}

//______________________________________________________________________________
void TG4RootDetectorConstruction::CreateG4LogicalVolumes()
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
void TG4RootDetectorConstruction::CreateG4PhysicalVolumes()
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
void TG4RootDetectorConstruction::CreateG4Materials()
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
void TG4RootDetectorConstruction::CreateG4Elements()
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
G4LogicalVolume *TG4RootDetectorConstruction::CreateG4LogicalVolume(TGeoVolume *vol)
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
      G4Exception("TG4RootDetectorConstruction::CreateG4LogicalVolume",
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
      G4Exception("TG4RootDetectorConstruction::CreateG4LogicalVolume",
                  "G4Root_F003", FatalException, description);
   }
   pVolume = new G4LogicalVolume(pSolid, pMaterial, sname,
                                                  NULL, NULL, NULL, false);
   fG4VolumeMap.insert(G4VolumeVal_t(vol, pVolume));
   fVolumeMap.insert(VolumeVal_t(pVolume, vol));
   return pVolume;
}

//______________________________________________________________________________
G4VPhysicalVolume *TG4RootDetectorConstruction::CreateG4PhysicalVolume(TGeoNode *node)
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
      G4Exception("TG4RootDetectorConstruction::CreateG4PhysicalVolume",
                  "G4Root_F004", FatalException, description);
   }
   G4LogicalVolume *pMotherLogical = CreateG4LogicalVolume(node->GetMotherVolume());
   if (!pMotherLogical && node!=fGeometry->GetTopNode()) {
      G4ExceptionDescription description;
      description << "      "
         << "No G4 mother volume crated for TGeo node " << node->GetName();
      G4Exception("TG4RootDetectorConstruction::CreateG4PhysicalVolume",
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
G4Material *TG4RootDetectorConstruction::CreateG4Material(const TGeoMaterial *mat)
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
            G4Exception("TG4RootDetectorConstruction::CreateG4Material",
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
G4RotationMatrix *TG4RootDetectorConstruction::CreateG4Rotation(const TGeoMatrix *matrix)
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
G4VSolid *TG4RootDetectorConstruction::CreateG4Solid(TGeoShape *shape)
{
/// Create a G4 generic solid working with any TGeo shape. If already created
/// return just a pointer to the existing one.
   return new TG4RootSolid(shape);
   return NULL;
}

//______________________________________________________________________________
G4Material *TG4RootDetectorConstruction::GetG4Material(const TGeoMaterial *mat) const
{
/// Retreive a G4 material mapped to a ROOT material.
   G4MaterialIt_t it = fG4MaterialMap.find(mat);
   if (it != fG4MaterialMap.end()) return it->second;
   return NULL;
}

//______________________________________________________________________________
G4LogicalVolume *TG4RootDetectorConstruction::GetG4Volume(const TGeoVolume *vol) const
{
/// Retreive a G4 logical volume mapped to a ROOT volume.
   G4VolumeIt_t it = fG4VolumeMap.find(vol);
   if (it != fG4VolumeMap.end()) return it->second;
   return NULL;
}

//______________________________________________________________________________
TGeoVolume *TG4RootDetectorConstruction::GetVolume(const G4LogicalVolume *g4vol) const
{
/// Retreive a TGeo logical volume mapped to a G4 volume.
   VolumeIt_t it = fVolumeMap.find(g4vol);
   if (it != fVolumeMap.end()) return it->second;
   return NULL;
}

//______________________________________________________________________________
G4VPhysicalVolume *TG4RootDetectorConstruction::GetG4VPhysicalVolume(const TGeoNode *node) const
{
/// Retreive a G4 physical volume mapped to a ROOT node.
   G4PVolumeIt_t it = fG4PVolumeMap.find(node);
   if (it != fG4PVolumeMap.end()) return it->second;
   return NULL;
}

//______________________________________________________________________________
TGeoNode *TG4RootDetectorConstruction::GetNode(const G4VPhysicalVolume *g4pvol) const
{
/// Retreive a TGeo node mapped to a G4 physical volume.
   PVolumeIt_t it = fPVolumeMap.find(g4pvol);
   if (it != fPVolumeMap.end()) return it->second;
   return NULL;
}

void TG4RootDetectorConstruction::AddPotentialSD(o2::base::Detector* detector)
{
  fPotentialSDs.push_back(detector);
}
