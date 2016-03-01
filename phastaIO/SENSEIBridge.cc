#include "SENSEIBridgeAPIMangling.h"

#include "SENSEIBridge.h"
#include "SENSEIAdaptor.h"
#include <ConfigurableAnalysis.h>

namespace BridgeInternals
{
  static vtkSmartPointer<sensei::SENSEIAdaptor> GlobalDataAdaptor;
  static vtkSmartPointer<sensei::ConfigurableAnalysis> GlobalAnalysisAdaptor;
}

void senseiinitialize()
{
  if (!BridgeInternals::GlobalDataAdaptor)
    {
    BridgeInternals::GlobalDataAdaptor = vtkSmartPointer<sensei::SENSEIAdaptor>::New();
    }
}

void senseiinitializegrid(int* numPoints, double* coordsArray, int* numCells)
{
  BridgeInternals::GlobalDataAdaptor->InitializeGrid(*numPoints, coordsArray, *numCells);
}

void senseiaddcbi(
  int* numCellsInBlock, int* numPointsPerCell, int* cellConnectivity)
{
  BridgeInternals::GlobalDataAdaptor->AddCellBlockInformation(
    *numCellsInBlock, *numPointsPerCell, cellConnectivity);
}

void senseiaddfieldinformation(int* nshg, double* dofArray, int* compressibleFlow)
{
  BridgeInternals::GlobalDataAdaptor->AddFieldInformation(
    *nshg, dofArray, *compressibleFlow != 0);
}

void senseiupdate(int* tstep, double* time)
{
  BridgeInternals::GlobalDataAdaptor->SetDataTime(*time);
  BridgeInternals::GlobalDataAdaptor->SetDataTimeStep(*tstep);
  BridgeInternals::GlobalDataAdaptor->AddArray(NULL, vtkDataObject::FIELD_ASSOCIATION_POINTS, "pressure");
  BridgeInternals::GlobalAnalysisAdaptor->Execute(BridgeInternals::GlobalDataAdaptor);
}

void senseireleasedata()
{
  BridgeInternals::GlobalDataAdaptor->ReleaseData();
}

void senseifinalize()
{
  BridgeInternals::GlobalAnalysisAdaptor = NULL;
  BridgeInternals::GlobalDataAdaptor = NULL;
}
