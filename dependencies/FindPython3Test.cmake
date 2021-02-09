# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

find_package(Python3 COMPONENTS Development.Embed)
message("Hello")
if(NOT Python3_FOUND)
  return()
endif()
message("Hello2")

#add_library(Python3::Python IMPORTED INTERFACE)

set_target_properties(Python3::Python
                      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                 "${Python3_INCLUDE_DIRS}")
set_target_properties(Python3::Python
                      PROPERTIES LINK_FLAGS
                                 "${Python3_LINK_OPTIONS}")

#list(GET Geant4_INCLUDE_DIRS 0 Geant4_INCLUDE_DIR)
#set(Geant4_LIBRARY_DIRS)
#foreach(gl4lib IN LISTS Geant4_LIBRARIES)
#  find_library(gl4libpath NAMES ${gl4lib} PATHS "${Geant4_INCLUDE_DIR}/../.."
#               PATH_SUFFIXES lib lib64
#               NO_DEFAULT_PATH)
#  if(gl4libpath)
#    get_filename_component(gl4libdir ${gl4libpath} DIRECTORY)
#    list(APPEND Geant4_LIBRARY_DIRS ${gl4libdir})
#  endif()
#  unset(gl4libpath CACHE)
#endforeach()
#list(REMOVE_DUPLICATES Geant4_LIBRARY_DIRS)
#set_target_properties(geant4
#                      PROPERTIES INTERFACE_LINK_DIRECTORIES
#                      "${Geant4_LIBRARY_DIRS}")

# Promote the imported target to global visibility
# (so we can alias it)
set_target_properties(Python3::Python PROPERTIES IMPORTED_GLOBAL TRUE)

add_library(Py::Python3Test ALIAS Python3::Python)
set(Python3Test_FOUND TRUE)
