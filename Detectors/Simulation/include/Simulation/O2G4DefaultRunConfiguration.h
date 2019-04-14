#ifndef O2G4_DEFAULT_RUN_CONFIGURATION_H
#define O2G4_DEFAULT_RUN_CONFIGURATION_H

//------------------------------------------------
// The Geant4 Virtual Monte Carlo package
// Copyright (C) 2007 - 2015 Ivana Hrivnacova
// All rights reserved.
//
// For the licensing terms see geant4_vmc/LICENSE.
// Contact: root-vmc@cern.ch
//-------------------------------------------------

/// \file O2G4RunConfiguration.h
/// \brief Definition of the O2G4RunConfiguration class
///
/// \author I. Hrivnacova; IPN Orsay


#include <memory>

#include "DetectorsBase/Detector.h"

#include "VO2G4RunConfiguration.h"

class G4VUserDetectorConstruction;
class G4VUserPrimaryGeneratorAction;
class G4VUserPhysicsList;
class G4VModularPhysicsList;
class G4UserRunAction;
class G4UserEventAction;
class G4UserStackingAction;
class G4RunManager;
class G4UImessenger;

class O2G4DetectorConstruction;

/// \ingroup run
/// \brief Takes care of creating Geant4 user action classes using VMC
///
/// This class takes care of creating all Geant4 user defined mandatory
/// and action classes that will be initialized and managed by Geant4
/// kernel (G4RunManager).
///
/// The class can be extended in a user application by inheritence;
/// this gives a user possibility to extend each Geant4 user defined class.
///
/// In constructor user has to specify the geometry input
/// and select geometry navigation via the following options:
/// - geomVMCtoGeant4   - geometry defined via VMC, G4 native navigation
/// - geomVMCtoRoot     - geometry defined via VMC, Root navigation
/// - geomRoot          - geometry defined via Root, Root navigation
/// - geomRootToGeant4  - geometry defined via Root, G4 native navigation
/// - geomGeant4        - geometry defined via Geant4, G4 native navigation
///
/// The second argument in the constructor selects physics list:
/// - emStandard         - standard em physics (default)
/// - emStandard+optical - standard em physics + optical physics
/// - XYZ                - selected hadron physics list ( XYZ = LHEP, QGSP, ...)
/// - XYZ+optical        - selected hadron physics list + optical physics
///
/// The third argument activates the special processes in the TG4SpecialPhysicsList,
/// which implement VMC features:
/// - stepLimiter       - step limiter (default)
/// - specialCuts       - VMC cuts
/// - specialControls   - VMC controls for activation/inactivation selected processes
/// - stackPopper       - stackPopper process
/// When more than one options are selected, they should be separated with '+'
/// character: eg. stepLimit+specialCuts.
///
/// \author I. Hrivnacova; IPN, Orsay

class O2G4DefaultRunConfiguration : public VO2G4RunConfiguration
{
  public:
    O2G4DefaultRunConfiguration();
    virtual ~O2G4DefaultRunConfiguration() = default;

    /// Not implemented
    O2G4DefaultRunConfiguration(const O2G4DefaultRunConfiguration&) = delete;
    O2G4DefaultRunConfiguration& operator=(const O2G4DefaultRunConfiguration&) = delete;

    //
    // methods
    //

    //
    // inherited
    //
    EExitStatus Initialize() override final;

    EExitStatus AddThread() override final;


    G4VUserDetectorConstruction* CreateDetectorConstruction() const override final;
    G4VUserPhysicsList* CreatePhysicsList() const override final;

    G4VUserPrimaryGeneratorAction* CreatePrimaryGeneratorAction() const override final;
    G4UserRunAction* CreateMasterRunAction() const override final;
    G4UserRunAction* CreateWorkerRunAction() const override final;
    G4UserEventAction* CreateEventAction() const override final;
    G4UserTrackingAction* CreateTrackingAction() const override final;
    G4UserSteppingAction* CreateSteppingAction() const override final;
    G4UserStackingAction* CreateStackingAction() const override final;

    G4UserWorkerInitialization* CreateWorkerInitialization() override final;

    G4Navigator* CreateNavigatorForTracking() const override final;

    //TG4VUserRegionConstruction*   CreateUserRegionConstruction();
    //TG4VUserPostDetConstruction*  CreateUserPostDetConstruction();
    //TG4VUserFastSimulation*       CreateUserFastSimulation();

    // set methods
    //void  SetMTApplication(Bool_t mtApplication);
    //void  SetParameter(const TString& name, Double_t value);

    // get methods
    //TString  GetUserGeometry() const;
    //TString  GetPhysicsListSelection() const;
    //Bool_t   IsSpecialStacking() const;
    //Bool_t   IsSpecialControls() const;
    //Bool_t   IsSpecialCuts() const;
    //Bool_t   IsMTApplication() const;

  protected:
    O2G4DetectorConstruction* fDetectorConstruction;
    G4VModularPhysicsList* fPhysicsList;
    TGeoManager* fGeoManager;


    //TString        fPhysicsListSelection;   ///< physics list selection
    //TString        fSpecialProcessSelection;///< special process selection
    //Bool_t         fSpecialStacking;        ///< option for special stacking
    //Bool_t         fMTApplication;          ///< option for MT mode if available
    //Bool_t         fSpecialControls;        ///< option for special controls
    //Bool_t         fSpecialCuts;            ///< option for special cuts
    //G4UImessenger* fAGDDMessenger;          //!< XML messenger
    //G4UImessenger* fGDMLMessenger;          //!< XML messenger

    /// The map of special parameters which need to be set before creating TGeant4
    /// Actually used for monopole properties:
    /// monopoleMass, monopoleElCharge, monopoleMagCharge
    //std::map<TString, Double_t>  fParameters; //!

};


#endif // O2G4_DEFAULT_RUN_CONFIGURATION_H
