// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file Detector.h
/// \brief Definition of the DetectorConstruction class

#ifndef ALICEO2_BASE_DETECTOR_H_
#define ALICEO2_BASE_DETECTOR_H_

#include <map>
#include <vector>
#include <initializer_list>
#include <memory>

#include "FairDetector.h"  // for FairDetector
#include "FairRootManager.h"
#include "DetectorsBase/MaterialManager.h"
#include "Rtypes.h"        // for Float_t, Int_t, Double_t, Detector::Class, etc
#include <cxxabi.h>
#include <typeinfo>
#include <type_traits>
#include <string>
#include <TMessage.h>
#include "CommonUtils/ShmManager.h"
#include "CommonUtils/ShmAllocator.h"
#include <sys/shm.h>
#include <type_traits>
#include <unistd.h>
#include <cassert>

#include <G4Step.hh>
#include <G4TouchableHistory.hh>
#include <TGeoVolume.h>
#include <TGeoManager.h>
#include <FairGeoNode.h>
#include <FairVolume.h>
#include <TString.h>
#include <TRefArray.h>


class FairMQParts;
class FairMQChannel;

namespace o2 {
namespace base {


class SensitiveDetector : public G4VSensitiveDetector
{
  public:
    SensitiveDetector();

    // returning the name of the branch (corresponding to probe)
    // returns zero length string when probe not defined
    virtual std::string getHitBranchNames(int probe) const = 0;

    // interface to update track indices of data objects
    // usually called by the Stack, at the end of an event, which might have changed
    // the track indices due to filtering
    // FIXME: make private friend of stack?
    virtual void updateHitTrackIndices(std::map<int, int> const&) = 0;

    // interfaces to attach properly encoded hit information to a FairMQ message
    // and to decode it
    virtual void attachHits(FairMQChannel&, FairMQParts&) = 0;
    virtual void fillHitBranch(TTree& tr, FairMQParts& parts, int& index) = 0;

    // interface needed to merge together hit entries in TBranches (as used by hit merger process)
    // the 2 maps given are
    // a) entries: a map giving for each real event; the entries in origin that contribute to this event
    // b) trackoffsets: a map giving the corresponding trackoffset to be applied to the trackID property when
    // merging
    virtual void mergeHitEntries(TTree& origin, TTree& target, std::map<int, std::vector<int>> const& entries,
                                 std::map<int, std::vector<int>> const& trackoffsets) = 0;

    /// Therefore force implementation of
    ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist, TGeoNode* currentNode) = 0;

  protected:
    SensitiveDetector& operator=(SensitiveDetector const&);

  private:
    SensitiveDetector();
    SensitiveDetector(SensitiveDetector const&);

    ClassDefOverride(SensitiveDetector, 0);
};

/// utility function to demangle cxx type names
inline std::string demangle(const char* name)
{
  int status = -4; // some arbitrary value to eliminate compiler warnings
  std::unique_ptr<char, void (*)(void*)> res{ abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free };
  return (status == 0) ? res.get() : name;
}

void attachShmMessage(void* hitsptr, FairMQChannel& channel, FairMQParts& parts, bool* busy_ptr);
void* decodeShmCore(FairMQParts& dataparts, int index, bool*& busy);

template <typename T>
T decodeShmMessage(FairMQParts& dataparts, int index, bool*& busy)
{
  return reinterpret_cast<T>(decodeShmCore(dataparts, index, busy));
}

// this goes into the source
void attachMessageBufferToParts(FairMQParts& parts, FairMQChannel& channel,
                                void* data, size_t size, void (*func_ptr)(void* data, void* hint), void* hint);

template <typename Container>
void attachTMessage(Container const& hits, FairMQChannel& channel, FairMQParts& parts)
{
  TMessage* tmsg = new TMessage();
  tmsg->WriteObjectAny((void*)&hits, TClass::GetClass(typeid(hits)));
  attachMessageBufferToParts(parts, channel, tmsg->Buffer(), tmsg->BufferSize(),
                             [](void* data, void* hint) { delete static_cast<TMessage*>(hint); }, tmsg);
}

void* decodeTMessageCore(FairMQParts& dataparts, int index);
template <typename T>
T decodeTMessage(FairMQParts& dataparts, int index)
{
  return static_cast<T>(decodeTMessageCore(dataparts, index));
}

void attachDetIDHeaderMessage(int id, FairMQChannel& channel, FairMQParts& parts);

template <typename T>
TBranch* getOrMakeBranch(TTree& tree, const char* brname, T* ptr)
{
  if (auto br = tree.GetBranch(brname)) {
    br->SetAddress(static_cast<void*>(&ptr));
    return br;
  }
  // otherwise make it
  return tree.Branch(brname, ptr);
}

// a trait to determine if we should use shared mem or serialize using TMessage
template <typename Det>
struct UseShm {
  static constexpr bool value = false;
};


template <typename Det>
class SDImpl : public o2::base::SensitiveDetector
{
 public:
  // offer same constructors as base
  using SensitiveDetector::SensitiveDetector;


  /// Override final G4VSensitiveDetector::ProcessHits and provide additionally the
  /// current TGeoNode
  bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) final
  {
    static_cast<Det*>(this)->ProcessHits(aStep, ROhist, fGeoManager->GetCurrentNode());
  }

  // default implementation for getHitBranchNames
  std::string getHitBranchNames(int probe) const override
  {
    return std::string();
  }
  /*
    if (probe == 0) {
      return addNameTo("Hit");
    }
    return std::string(); // empty string as undefined
  }*/

  // generic implementation for the updateHitTrackIndices interface
  // assumes Detectors have a GetHits(int) function that return some iterable
  // hits which are o2::BaseHits
  void updateHitTrackIndices(std::map<int, int> const& indexmapping) override
  {
    return;
  }
  /*
    int probe = 0; // some Detectors have multiple hit vectors and we are probing
                   // them via a probe integer until we get a nullptr
    while (auto hits = static_cast<Det*>(this)->Det::getHits(probe++)) {
      for (auto& hit : *hits) {
        auto iter = indexmapping.find(hit.GetTrackID());
        hit.SetTrackID(iter->second);
      }
    }
  }*/

  void attachHits(FairMQChannel& channel, FairMQParts& parts) override
  {
    return;
  }
  /*
    int probe = 0;
    // check if there is anything to be attached
    // at least the first hit index should return non nullptr
    if (static_cast<Det*>(this)->Det::getHits(0) == nullptr) {
      return;
    }

    attachDetIDHeaderMessage(GetDetId(), channel, parts); // the DetId s are universal as they come from o2::detector::DetID

    while (auto hits = static_cast<Det*>(this)->Det::getHits(probe++)) {
      if (!UseShm<Det>::value || !o2::utils::ShmManager::Instance().isOperational()) {
        attachTMessage(*hits, channel, parts);
      } else {
        // this is the shared mem variant
        // we will just send the sharedmem ID and the offset inside
        *mShmBusy[mCurrentBuffer] = true;
        attachShmMessage((void*)hits, channel, parts, mShmBusy[mCurrentBuffer]);
      }
    }
  }*/

  // this merges several entries from the TBranch brname from the origin TTree
  // into a single entry in a target TTree / same branch
  // (assuming T is typically a vector; merging is simply done by appending)
  // make this function a (static helper)
  template <typename T>
  void mergeAndAdjustHits(std::string const& brname, TTree& origin, TTree& target,
                          std::map<int, std::vector<int>> const& entrygroups,
                          std::map<int, std::vector<int>> const& trackoffsets)
  {
    return;
  }
  /*
    auto originbr = origin.GetBranch(brname.c_str());
    if (originbr) {
      auto targetdata = new T;
      T* incomingdata = nullptr;
      originbr->SetAddress(&incomingdata);

      T* filladdress;
      for (auto& event_entries_pair : entrygroups) {
        const auto& entries = event_entries_pair.second;
        int offset = 0;
        const auto& offsetvector = trackoffsets.find(event_entries_pair.first)->second;
        int entrycounter = 0;
        if (entries.size() == 1) {
          originbr->GetEntry(entries[0]);
          filladdress = incomingdata;
        } else {
          for (auto& e : entries) {
            filladdress = targetdata;
            originbr->GetEntry(e);
            if (incomingdata) {
              if (offset != 0) {
                // fix the trackIDs for this data
                for (auto& hit : *incomingdata) {
                  const auto oldID = hit.GetTrackID();
                  hit.SetTrackID(oldID + offset);
                }
              }
              // this could be further generalized by using a policy for T
              std::copy(incomingdata->begin(), incomingdata->end(), std::back_inserter(*targetdata));
              // adjust offset
              offset += offsetvector[entrycounter];
              delete incomingdata;
              incomingdata = nullptr;
            }
            entrycounter++;
          }
        }
        // fill target for this event
        auto targetbr = o2::base::getOrMakeBranch(target, brname.c_str(), &filladdress);
        targetbr->SetAddress(&filladdress);
        targetbr->Fill();
        targetbr->ResetAddress();
        targetdata->clear();
        if (incomingdata) {
          delete incomingdata;
          incomingdata = nullptr;
        }
      }
      delete targetdata;
    }
  }*/

  void mergeHitEntries(TTree& origin, TTree& target, std::map<int, std::vector<int>> const& entries,
                       std::map<int, std::vector<int>> const& trackoffsets) final
  {
    return;
  }
  /*
    // loop over hit containers / different branches
    // adjust trackID in hits on the go
    int probe = 0;
    using Hit_t = decltype(static_cast<Det*>(this)->Det::getHits(probe));
    std::string name = static_cast<Det*>(this)->getHitBranchNames(probe++);
    while (name.size() > 0) {
      mergeAndAdjustHits<typename std::remove_pointer<Hit_t>::type>(name, origin, target, entries, trackoffsets);
      // next name
      name = static_cast<Det*>(this)->getHitBranchNames(probe++);
    }
  }*/

 public:
  void fillHitBranch(TTree& tr, FairMQParts& parts, int& index) override
  {
    return;
  }
  /*
    int probe = 0;
    using Hit_t = decltype(static_cast<Det*>(this)->Det::getHits(probe));
    std::string name = static_cast<Det*>(this)->getHitBranchNames(probe++);
    while (name.size() > 0) {
      if (!UseShm<Det>::value || !o2::utils::ShmManager::Instance().isOperational()) {

        // for each branch name we extract/decode hits from the message parts ...
        auto hitsptr = decodeTMessage<Hit_t>(parts, index++);
        if (hitsptr) {
          // ... and fill the tree branch
          auto br = getOrMakeBranch(tr, name.c_str(), hitsptr);
          br->SetAddress(static_cast<void*>(&hitsptr));
          br->Fill();
          br->ResetAddress();
          delete hitsptr;
        }
      } else {
        // for each branch name we extract/decode hits from the message parts ...
        bool* busy;
        auto hitsptr = decodeShmMessage<Hit_t>(parts, index++, busy);
        LOG(INFO) << "GOT " << hitsptr->size() << " HITS ";
        // ... and fill the tree branch
        auto br = getOrMakeBranch(tr, name.c_str(), hitsptr);
        br->SetAddress(static_cast<void*>(&hitsptr));
        br->Fill();
        br->ResetAddress();

        // he we are done so unset the busy flag
        *busy = false;
      }
      // next name
      name = static_cast<Det*>(this)->getHitBranchNames(probe++);
    }
  }*/

  void freeHitBuffers()
  {
    return;
  }
  /*
    using Hit_t = decltype(static_cast<Det*>(this)->Det::getHits(0));
    if (UseShm<Det>::value) {
      for (int buffer = 0; buffer < NHITBUFFERS; ++buffer) {
        for (auto ptr : mCachedPtr[buffer]) {
          o2::utils::freeSimVector(static_cast<Hit_t>(ptr));
        }
      }
    }
  }*/

  template <typename Hit_t>
  bool setHits(int i, std::vector<Hit_t>* ptr)
  {
    return false;
  }
  /*
    if (i == 0) {
      static_cast<Det*>(this)->Det::mHits = ptr;
    }
    return false;
  }*/

  // creating a number of hit buffers (in shared mem) -- to which
  // detectors can write in round-robin fashion
  void createHitBuffers()
  {
    return;
  }
  /*
    using VectorHit_t = decltype(static_cast<Det*>(this)->Det::getHits(0));
    using Hit_t = typename std::remove_pointer<VectorHit_t>::type::value_type;
    for (int buffer = 0; buffer < NHITBUFFERS; ++buffer) {
      int probe = 0;
      bool more{ false };
      do {
        auto ptr = o2::utils::createSimVector<Hit_t>();
        more = static_cast<Det*>(this)->Det::setHits(probe, ptr);
        mCachedPtr[buffer].emplace_back(ptr);
        probe++;
      } while (more);
    }
  }*/

  void initializeLate() final
  {
    if (!mInitialized) {
      if (UseShm<Det>::value) {
        static_cast<Det*>(this)->Det::createHitBuffers();
        for (int b = 0; b < NHITBUFFERS; ++b) {
          auto& instance = o2::utils::ShmManager::Instance();
          mShmBusy[b] = instance.hasSegment() ? (bool*)instance.getmemblock(sizeof(bool)) : new bool;
          *mShmBusy[b] = false;
        }
      }
      mInitialized = true;
      mCurrentBuffer = 0;
    }
  }


  void BeginEvent() final
  {
    return;
  }/*
    if (UseShm<Det>::value) {
      mCurrentBuffer = (mCurrentBuffer + 1) % NHITBUFFERS;
      while (mShmBusy[mCurrentBuffer] != nullptr && *mShmBusy[mCurrentBuffer]) {
        // this should ideally never happen
        LOG(INFO) << " BUSY WAITING SIZE ";
        sleep(1);
      }

      using Hit_t = decltype(static_cast<Det*>(this)->Det::getHits(0));

      // now we have to clear the hits before writing again
      int probe = 0;
      for (auto bareptr : mCachedPtr[mCurrentBuffer]) {
        auto hits = static_cast<Hit_t>(bareptr);
        // assign ..
        static_cast<Det*>(this)->Det::setHits(probe, hits);
        hits->clear();
        probe++;
      }
    }
  }*/

  ~SDImpl() override
  {
    return;
  }
  /*
    for (int i = 0; i < NHITBUFFERS; ++i) {
      if (mShmBusy[i]) {
        auto& instance = o2::utils::ShmManager::Instance();
        if (instance.hasSegment()) {
          instance.freememblock(mShmBusy[i]);
        } else {
          delete mShmBusy[i];
        }
      }
    }
    freeHitBuffers();
  }*/

 protected:
  static constexpr int NHITBUFFERS = 3;      // number of buffers for hits in order to allow async processing
                                             // in the hit merger without blocking nor copying the data
                                             // (like done in typical data aquisition systems)
  bool* mShmBusy[NHITBUFFERS] = { nullptr }; //! pointer to bool in shared mem indicating of IO busy
  std::vector<void*> mCachedPtr[NHITBUFFERS];
  int mCurrentBuffer = 0; // holding the current buffer information
  int mInitialized = false;
  ClassDefOverride(SDImpl, 0);
};

}
}

#endif
