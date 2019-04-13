// @(#)root/g4root:$Id$
// Author: Andrei Gheata   07/08/06

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/// \file O2G4DetectorConstruction.h
/// \brief Definition of the O2G4DetectorConstruction and
///        TVirtualUserPostDetConstruction classes
///
/// \author A. Gheata; CERN

#ifndef ROOT_O2G4_DETECTOR_CONSTRUCTION
#define ROOT_O2G4_DETECTOR_CONSTRUCTION


#ifndef G4VUserDetectorConstruction_h
#include "G4VUserDetectorConstruction.hh"
#endif

#ifndef G4ROTATIONMATRIX_HH
#include "G4RotationMatrix.hh"
#endif

#ifndef ROOT_TGeoManager
#include "TGeoManager.h"
#endif

#ifndef ROOT_TGeoNode
#include "TGeoNode.h"
#endif

#include <map>
#include <vector>

#include "DetectorsBase/Detector.h"

#include "FairModule.h"

#include "O2SimInterface.h"


class TObjArray;
class TGeoManager;
class TGeoMaterial;
class TGeoShape;
class TGeoVolume;
class TGeoMatrix;

class G4FieldManager;
class G4VSolid;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;

/// \brief Builder creating a pseudo G4 geometry starting from a TGeo geometry.
///
/// To invoke the method Construct() the ROOT geometry must be in memory.
/// The G4 objects created are:
///  - TGeoElement               ---> G4Element
///  - TGeoMaterial/TGeoMixture  ---> G4Material
///  - TGeoMatrix                ---> G4RotationMatrix
///  - TGeoVolume                ---> G4LogicalVolume
///  - TGeoShape           ---> TG4RootSolid  : public G4Solid
///  - TGeoNode            ---> G4PVPlacement : public G4VPhysicalVolume
///
/// \author A. Gheata; CERN

class O2G4DetectorConstruction : public G4VUserDetectorConstruction, public O2SimInterface {

private:
   /// the map from TGeoMaterial to G4Material
   typedef std::map<const TGeoMaterial *, G4Material *>   G4MaterialMap_t;
   /// the constant iterator for the map from TGeoMaterial to G4Material
   typedef G4MaterialMap_t::const_iterator            G4MaterialIt_t;
   /// the value type for the map from TGeoMaterial to G4Material
   typedef G4MaterialMap_t::value_type                G4MaterialVal_t;
   G4MaterialMap_t       fG4MaterialMap; //!< map of G4 materials

   /// the map from TGeoVolume to G4LogicalVolume
   typedef std::map<const TGeoVolume *, G4LogicalVolume *> G4VolumeMap_t;
   /// the constant iterator for the map from TGeoVolume to G4LogicalVolume
   typedef G4VolumeMap_t::const_iterator              G4VolumeIt_t;
   /// the value type for the map from TGeoVolume to G4LogicalVolume
   typedef G4VolumeMap_t::value_type                  G4VolumeVal_t;
   G4VolumeMap_t         fG4VolumeMap; //!< map of G4 volumes

   /// the map from G4LogicalVolume to TGeoVolume
   typedef std::map<const G4LogicalVolume *, TGeoVolume *> VolumeMap_t;
   /// the constant iterator for the map from G4LogicalVolume to TGeoVolume
   typedef VolumeMap_t::const_iterator                VolumeIt_t;
   /// the value type for the map from  G4LogicalVolume to TGeoVolume
   typedef VolumeMap_t::value_type                    VolumeVal_t;
   VolumeMap_t           fVolumeMap; //!< map of TGeo volumes

   /// the map from TGeoNode to G4VPhysicalVolume
   typedef std::map<const TGeoNode *, G4VPhysicalVolume *> G4PVolumeMap_t;
   /// the constant iterator for the map from TGeoNode to G4VPhysicalVolume
   typedef G4PVolumeMap_t::const_iterator             G4PVolumeIt_t;
   /// the value type for the map from TGeoNode to G4VPhysicalVolume
   typedef G4PVolumeMap_t::value_type                 G4PVolumeVal_t;
   G4PVolumeMap_t        fG4PVolumeMap; //!< map of G4 physical volumes

   /// the map from G4VPhysicalVolume to TGeoNode
   typedef std::map<const G4VPhysicalVolume *, TGeoNode *> PVolumeMap_t;
   /// the constant iterator for the map from G4VPhysicalVolume to TGeoNode
   typedef PVolumeMap_t::const_iterator               PVolumeIt_t;
   /// the value type for the map from G4VPhysicalVolume to TGeoNode
   typedef PVolumeMap_t::value_type                   PVolumeVal_t;
   PVolumeMap_t          fPVolumeMap; //!< map of TGeo volumes

   /// Holding FairModules which could be sensitive
   /// NOTE We don't know at this stage and this is for now a workaround
   //std::vector<o2::base::Detector*> fPotentialSDs;

   std::map<o2::base::Detector*,std::map<G4LogicalVolume*, TGeoVolume*>> fSVMap;
   std::vector<o2::base::Detector*> fSDs;
   std::vector<o2::base::Detector*> fSDClones;
   std::vector<FairModule*> fPassive;
   std::vector<FairModule*> fModules;
   // Map TGeoVolume ID to FairModule ID
   std::map<G4int,G4int> fModVolMap;


protected:
   G4bool               fIsConstructed;   ///< flag Construct() called
   TGeoManager          *fGeometry;        ///< TGeo geometry manager
   G4VPhysicalVolume    *fTopPV;           ///< World G4 physical volume
   // Geometry creators
   void                  CreateG4LogicalVolumes();
   void                  CreateG4Materials();
   void                  CreateG4Elements();
   void                  CreateG4PhysicalVolumes();
   // Converters TGeo->G4 for basic types
   G4VSolid             *CreateG4Solid(TGeoShape *shape);
   G4LogicalVolume      *CreateG4LogicalVolume(TGeoVolume *vol);
   G4VPhysicalVolume    *CreateG4PhysicalVolume(TGeoNode *node);
   G4Material           *CreateG4Material(const TGeoMaterial *mat);
   G4RotationMatrix     *CreateG4Rotation(const TGeoMatrix *matrix);

   void ExtractSensitiveVolumes(o2::base::Detector* detector);


public:
   O2G4DetectorConstruction();
   O2G4DetectorConstruction(TGeoManager *geom);
   virtual ~O2G4DetectorConstruction();

   G4VPhysicalVolume *Construct() override final;
   void               ConstructSDandField() override final;
   void AddModule(FairModule* module);
   EExitStatus Initialize() override;


   // Getters
                         /// Return TGeo geometry manager
   TGeoManager          *GetGeometryManager() const {return fGeometry;}
                         /// Return the World G4 physical volume
   G4VPhysicalVolume    *GetTopPV()           const {return fTopPV;}
   G4Material           *GetG4Material(const TGeoMaterial *mat)  const;
   G4LogicalVolume      *GetG4Volume(const TGeoVolume *vol)      const;
   TGeoVolume           *GetVolume(const G4LogicalVolume *g4vol) const;
   G4VPhysicalVolume    *GetG4VPhysicalVolume(const TGeoNode *node) const;
   TGeoNode             *GetNode(const G4VPhysicalVolume *g4vol) const;
                         /// Return the sensitive detector hook
                         /// Return the flag Construct() called
   G4bool                IsConstructed() const {return fIsConstructed;}


//   ClassDef(O2G4DetectorConstruction,0)  // Class creating a G4 gometry based on ROOT geometry
};

#endif /* ROOT_O2G4_DETECTOR_CONSTRUCTION */
