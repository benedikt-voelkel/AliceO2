// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef ALICEO2_TOF_DIGIT_H_
#define ALICEO2_TOF_DIGIT_H_

#include <CommonDataFormat/TimeStamp.h>
#include <iosfwd>
#include "Rtypes.h"

#include <boost/serialization/base_object.hpp> // for base_object

namespace o2 {
namespace tof {
/// \class Digit
/// \brief TOF digit implementation
class Digit : public o2::dataformats::TimeStamp<double>
{
 public:
  Digit() = default;

  Digit(Double_t time, Int_t channel, Int_t tdc, Int_t tot, Int_t bc);
  ~Digit() = default;

  Int_t getChannel() const { return mChannel; }
  void setChannel(Int_t channel) { mChannel = channel; }

  Int_t getTDC() const { return mTDC; }
  void setTDC(Int_t tdc) { mTDC = tdc; }

  Int_t getTOT() const { return mTOT; }
  void setTOT(Int_t tot) { mTOT = tot; }

  Int_t getBC() const { return mBC; }
  void setBC(Int_t bc) { mBC = bc; }

  void printStream(std::ostream &stream) const;

private:
  friend class boost::serialization::access;

  Int_t mChannel;       ///< TOF channel index
  Int_t mTDC;           ///< TDC bin number
  Int_t mTOT;           ///< TOT bin number
  Int_t mBC;            ///< Bunch Crossing

  ClassDefNV(Digit, 1);
};

std::ostream &operator<<(std::ostream &stream, const Digit &dig);
} // namespace TOF
} // namespace o2
#endif
