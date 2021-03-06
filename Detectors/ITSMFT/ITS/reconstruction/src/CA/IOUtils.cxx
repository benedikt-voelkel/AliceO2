// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
///
/// \file IOUtils.cxx
/// \brief
///

#include "ITSReconstruction/CA/IOUtils.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <tuple>
#include <unordered_set>
#include <utility>

#include "DataFormatsITSMFT/Cluster.h"
#include "ITSBase/GeometryTGeo.h"
#include "ITSReconstruction/CA/Constants.h"
#include "MathUtils/Utils.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCTruthContainer.h"

namespace
{
constexpr int PrimaryVertexLayerId{ -1 };
constexpr int EventLabelsSeparator{ -1 };
} // namespace

namespace o2
{
namespace ITS
{
namespace CA
{

std::vector<Event> IOUtils::loadEventData(const std::string& fileName)
{
  std::vector<Event> events{};
  std::ifstream inputStream{};
  std::string line{}, unusedVariable{};
  int layerId{}, monteCarlo{};
  int clusterId{ EventLabelsSeparator };
  float xCoordinate{}, yCoordinate{}, zCoordinate{}, alphaAngle{};
  float varZ{ -1.f }, varY{ -1.f };

  inputStream.open(fileName);

  /// THIS IS LEAKING IN THE BACKWARD COMPATIBLE MODE. KEEP IT IN MIND.
  dataformats::MCTruthContainer<MCCompLabel>* mcLabels = nullptr;
  while (std::getline(inputStream, line)) {

    std::istringstream inputStringStream(line);
    if (inputStringStream >> layerId >> xCoordinate >> yCoordinate >> zCoordinate) {

      if (layerId == PrimaryVertexLayerId) {

        if (clusterId != 0) {
          events.emplace_back(events.size());
        }

        events.back().addPrimaryVertex(xCoordinate, yCoordinate, zCoordinate);
        clusterId = 0;

      } else {

        if (inputStringStream >> varY >> varZ >> unusedVariable >> alphaAngle >> monteCarlo) {
          events.back().addClusterToLayer(layerId, xCoordinate, yCoordinate, zCoordinate,
                                          events.back().getLayer(layerId).getClustersSize());
          const float sinAlpha = std::sin(alphaAngle);
          const float cosAlpha = std::cos(alphaAngle);
          const float xTF = xCoordinate * cosAlpha - yCoordinate * sinAlpha;
          const float yTF = xCoordinate * sinAlpha + yCoordinate * cosAlpha;
          events.back().addTrackingFrameInfoToLayer(layerId, xTF, alphaAngle, std::array<float, 2>{ yTF, zCoordinate },
                                                    std::array<float, 3>{ varY, 0.f, varZ });
          events.back().addClusterLabelToLayer(layerId, MCCompLabel(monteCarlo));

          ++clusterId;
        }
      }
    }
  }

  return events;
}

void IOUtils::loadEventData(Event& event, const std::vector<ITSMFT::Cluster>* clusters,
                            const dataformats::MCTruthContainer<MCCompLabel>* mcLabels)
{
  if (!clusters) {
    std::cerr << "Missing clusters." << std::endl;
    return;
  }
  event.clear();
  GeometryTGeo* geom = GeometryTGeo::Instance();
  geom->fillMatrixCache(utils::bit2Mask(TransformType::T2GRot));
  int clusterId{ 0 };

  for (auto& c : *clusters) {
    int layer = geom->getLayer(c.getSensorID());

    /// Clusters are stored in the tracking frame
    event.addTrackingFrameInfoToLayer(layer, c.getX(), geom->getSensorRefAlpha(c.getSensorID()),
                                      std::array<float, 2>{ c.getY(), c.getZ() },
                                      std::array<float, 3>{ c.getSigmaY2(), c.getSigmaYZ(), c.getSigmaZ2() });

    /// Rotate to the global frame
    auto xyz = c.getXYZGloRot(*geom);
    event.addClusterToLayer(layer, xyz.x(), xyz.y(), xyz.z(), event.getLayer(layer).getClustersSize());
    event.addClusterLabelToLayer(layer, *(mcLabels->getLabels(clusterId).begin()));
    clusterId++;
  }
}

int IOUtils::loadROFrameData(std::uint32_t roFrame, Event& event, const std::vector<ITSMFT::Cluster>* clusters,
                             const dataformats::MCTruthContainer<MCCompLabel>* mcLabels)
{
  if (!clusters) {
    std::cerr << "Missing clusters." << std::endl;
    return -1;
  }
  event.clear();
  GeometryTGeo* geom = GeometryTGeo::Instance();
  geom->fillMatrixCache(utils::bit2Mask(TransformType::T2GRot));
  int clusterId{ 0 };
  int nused = 0;
  for (auto& c : *clusters) {
    if (c.getROFrame() == roFrame) {
      int layer = geom->getLayer(c.getSensorID());

      /// Clusters are stored in the tracking frame
      event.addTrackingFrameInfoToLayer(layer, c.getX(), geom->getSensorRefAlpha(c.getSensorID()),
                                        std::array<float, 2>{ c.getY(), c.getZ() },
                                        std::array<float, 3>{ c.getSigmaY2(), c.getSigmaYZ(), c.getSigmaZ2() });

      /// Rotate to the global frame
      auto xyz = c.getXYZGloRot(*geom);
      event.addClusterToLayer(layer, xyz.x(), xyz.y(), xyz.z(), event.getLayer(layer).getClustersSize());
      event.addClusterLabelToLayer(layer, *(mcLabels->getLabels(clusterId).begin()));
      nused++;
    }
    clusterId++;
  }
  return nused;
}

std::vector<std::unordered_map<int, Label>> IOUtils::loadLabels(const int eventsNum, const std::string& fileName)
{
  std::vector<std::unordered_map<int, Label>> labelsMap{};
  std::unordered_map<int, Label> currentEventLabelsMap{};
  std::ifstream inputStream{};
  std::string line{};
  int monteCarloId{}, pdgCode{}, numberOfClusters{};
  float transverseMomentum{}, phiCoordinate{}, pseudorapidity{};

  labelsMap.reserve(eventsNum);

  inputStream.open(fileName);
  std::getline(inputStream, line);

  while (std::getline(inputStream, line)) {

    std::istringstream inputStringStream(line);

    if (inputStringStream >> monteCarloId) {

      if (monteCarloId == EventLabelsSeparator) {

        labelsMap.emplace_back(currentEventLabelsMap);
        currentEventLabelsMap.clear();

      } else {

        if (inputStringStream >> transverseMomentum >> phiCoordinate >> pseudorapidity >> pdgCode >> numberOfClusters) {

          if (std::abs(pdgCode) == Constants::PDGCodes::PionCode && numberOfClusters == 7) {

            currentEventLabelsMap.emplace(std::piecewise_construct, std::forward_as_tuple(monteCarloId),
                                          std::forward_as_tuple(monteCarloId, transverseMomentum, phiCoordinate,
                                                                pseudorapidity, pdgCode, numberOfClusters));
          }
        }
      }
    }
  }

  labelsMap.emplace_back(currentEventLabelsMap);

  return labelsMap;
}

void IOUtils::writeRoadsReport(std::ofstream& correctRoadsOutputStream, std::ofstream& duplicateRoadsOutputStream,
                               std::ofstream& fakeRoadsOutputStream, const std::vector<std::vector<Road>>& roads,
                               const std::unordered_map<int, Label>& labelsMap)
{
  const int numVertices{ static_cast<int>(roads.size()) };
  std::unordered_set<int> foundMonteCarloIds{};

  correctRoadsOutputStream << EventLabelsSeparator << std::endl;
  fakeRoadsOutputStream << EventLabelsSeparator << std::endl;

  for (int iVertex{ 0 }; iVertex < numVertices; ++iVertex) {

    const std::vector<Road>& currentVertexRoads{ roads[iVertex] };
    const int numRoads{ static_cast<int>(currentVertexRoads.size()) };

    for (int iRoad{ 0 }; iRoad < numRoads; ++iRoad) {

      const Road& currentRoad{ currentVertexRoads[iRoad] };
      const int currentRoadLabel{ currentRoad.getLabel() };

      if (!labelsMap.count(currentRoadLabel)) {

        continue;
      }

      const Label& currentLabel{ labelsMap.at(currentRoadLabel) };

      if (currentRoad.isFakeRoad()) {

        fakeRoadsOutputStream << currentLabel << std::endl;

      } else {

        if (foundMonteCarloIds.count(currentLabel.monteCarloId)) {

          duplicateRoadsOutputStream << currentLabel << std::endl;

        } else {

          correctRoadsOutputStream << currentLabel << std::endl;
          foundMonteCarloIds.emplace(currentLabel.monteCarloId);
        }
      }
    }
  }
}
} // namespace CA
} // namespace ITS
} // namespace o2
