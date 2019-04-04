#ifndef O2G4_RUNMANAGER_H
#define O2G4_RUNMANAGER_H

#include <memory>
#include <vector>

#define PARENT_CLASS G4RunManagerMT
#ifdef G4MULTITHREADED
#include "G4RunManagerMT.hh"
#else
#include "G4RunManager.hh"
#define PARENT_CLASS G4RunManager
#endif



class G4VisExecutive;


/// \ingroup run
/// \brief Implementation of the TVirtualMC interface for Geant4.
///
/// \author I. Hrivnacova; IPN, Orsay

class O2G4RunManager : public PARENT_CLASS, public O2SimInterface
{
  public:
    O2G4RunManager(VO2G4RunConfiguration* configuration, int argc = 0, char** argv = 0);
    virtual ~O2G4RunManager() = default;

    O2G4RunManager() = delete;
    O2G4RunManager(const O2G4RunManager&) = delete;
    O2G4RunManager& operator=(const O2G4RunManager&) = delete;

    void Initialize() override final;

    //
    // Config
    //
    G4bool IsMT() const;


    //
    // methods for building/management of geometry
    // ------------------------------------------------
    //

  private:
    // set geometry from Root (built via TGeo)
    void SetGeometryInterface(VO2G4GeometryInterface* geometryInterface);
    std::vector<std::unique_ptr<G4Navigator>> fUserNavigators;

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
