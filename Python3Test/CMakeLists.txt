# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

o2_add_library(PyTest
    SOURCES src/PyWrap.cxx
    PUBLIC_LINK_LIBRARIES Py::Python3Test)



o2_add_executable(o2python
                  COMPONENT_NAME PyTest
                  PUBLIC_LINK_LIBRARIES O2::PyTest 
                  SOURCES run/o2python.cxx)



#o2_add_test(
#  DCAFitter
#  SOURCES test/testDCAFitter.cxx
#  COMPONENT_NAME DetectorsBase
#  PUBLIC_LINK_LIBRARIES O2::DetectorsBase
#  LABELS detectorsbase
#  ENVIRONMENT O2_ROOT=${CMAKE_BINARY_DIR}/stage
#  VMCWORKDIR=${CMAKE_BINARY_DIR}/stage/${CMAKE_INSTALL_DATADIR})

#o2_add_test_root_macro(test/buildMatBudLUT.C
#                       PUBLIC_LINK_LIBRARIES O2::DetectorsBase
#                       LABELS detectorsbase)
