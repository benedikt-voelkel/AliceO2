set(MODULE_NAME "Simulation")

O2_SETUP(NAME ${MODULE_NAME})

set(SRCS
  src/O2G4RunManager.cxx
  src/VO2G4RunConfiguration.cxx
  src/O2G4DefaultRunConfiguration.cxx
  src/O2G4EventAction.cxx
  src/O2G4PrimaryGeneratorAction.cxx
  src/O2G4RunAction.cxx
  src/O2G4SteppingAction.cxx
  src/O2G4TrackingAction.cxx
  src/O2G4WorkerInitialization.cxx
  #src/O2G4DetectorConstruction.cxx
  src/O2SimInterface.cxx
  src/TG4RootNavMgr.cxx
  src/TG4RootNavigator.cxx
  src/TG4RootDetectorConstruction.cxx
  src/TG4RootSolid.cxx
  src/TG4G3Units.cxx
  )

Set(HEADERS
  include/${MODULE_NAME}/O2G4RunManager.h
  include/${MODULE_NAME}/VO2G4RunConfiguration.h
  include/${MODULE_NAME}/O2G4DefaultRunConfiguration.h
  include/${MODULE_NAME}/O2G4EventAction.h
  include/${MODULE_NAME}/O2G4PrimaryGeneratorAction.h
  include/${MODULE_NAME}/O2G4RunAction.h
  include/${MODULE_NAME}/O2G4SteppingAction.h
  include/${MODULE_NAME}/O2G4TrackingAction.h
  include/${MODULE_NAME}/O2G4WorkerInitialization.h
  #include/${MODULE_NAME}/O2G4DetectorConstruction.h
  include/${MODULE_NAME}/O2SimInterface.h
  include/${MODULE_NAME}/TG4RootNavMgr.h
  include/${MODULE_NAME}/TG4RootNavigator.h
  include/${MODULE_NAME}/TG4RootDetectorConstruction.h
  include/${MODULE_NAME}/TG4RootSolid.h
  include/${MODULE_NAME}/TG4G3Units.h
)

#Set(LINKDEF src/DetectorsBaseLinkDef.h)
Set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME simulation_base_bucket)

O2_GENERATE_LIBRARY()
