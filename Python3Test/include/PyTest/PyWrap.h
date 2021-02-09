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
/// \brief Definition of the MaterialManager class

#ifndef ALICEO2_PYTHON_TEST_H_
#define ALICEO2_PYTHON_TEST_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>


namespace o2
{
namespace python
{
class PyWrap
{
 public:

  PyWrap() = default;
  ~PyWrap() = default;

  void run();

  void init() {;}

 //public:
  //ClassDefNV(MaterialManager, 0);
};

} // namespace python 
} // namespace o2

#endif
