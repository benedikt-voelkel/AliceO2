#ifndef O2G4_RUNMANAGER_H
#define O2G4_RUNMANAGER_H

#include <memory>
#include <vector>

#include <FairModule.h>

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
//#define PARENT_CLASS G4RunManagerMT
#else
#include "G4RunManager.hh"
//#define PARENT_CLASS G4RunManager
#endif

#include <G4Navigator.hh>

#include "O2SimInterface.h"
#include "VO2G4RunConfiguration.h"


class G4VisExecutive;

#ifdef G4MULTITHREADED
class O2G4RunManager : public G4MTRunManager
#else
class O2G4RunManager : public G4RunManager
#endif
{
  public:
    O2G4RunManager(VO2G4RunConfiguration* configuration);
    virtual ~O2G4RunManager() = default;

    O2G4RunManager() = delete;
    O2G4RunManager(const O2G4RunManager&) = delete;
    O2G4RunManager& operator=(const O2G4RunManager&) = delete;

    void RegisterNavigator(G4Navigator* userNavigator);

    //
    // Config
    //
    G4bool IsMT() const;


    //
    // methods for building/management of geometry
    // ------------------------------------------------
    //

  private:
    std::vector<std::unique_ptr<G4Navigator>> fUserNavigators;
    /// That is owned by the user
    VO2G4RunConfiguration* fRunConfiguration;

};

/*
#ifndef __CINT__
#ifndef __CLING__

// inline methods
#include "O2G4RunManager.icc"

#endif
#endif

*/

#undef PARENT_CLASS
#endif // O2_G4_RUNMANAGER_H
