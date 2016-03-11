#include "SENSEIBridgeAPIMangling.h"

#include "SENSEIBridge.h"
#include "SENSEIAdaptor.h"
#include <Autocorrelation.h>

namespace BridgeInternals
{
  static vtkSmartPointer<sensei::SENSEIAdaptor> GlobalDataAdaptor;
  static vtkSmartPointer<sensei::Autocorrelation> GlobalAnalysisAdaptor;
}

void senseiinitialize()
{
  if (!BridgeInternals::GlobalDataAdaptor)
    {
    BridgeInternals::GlobalDataAdaptor = vtkSmartPointer<sensei::SENSEIAdaptor>::New();
    }
  if (!BridgeInternals::GlobalAnalysisAdaptor)
    {
    BridgeInternals::GlobalAnalysisAdaptor = vtkSmartPointer<sensei::Autocorrelation>::New();
    std::string arrayname = "pressure";
    BridgeInternals::GlobalAnalysisAdaptor->Initialize(MPI_COMM_WORLD, 2, vtkDataObject::FIELD_ASSOCIATION_POINTS, arrayname, 4);
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
  vtkDataObject* mesh = BridgeInternals::GlobalDataAdaptor->GetMesh();
  BridgeInternals::GlobalDataAdaptor->AddArray(mesh, vtkDataObject::FIELD_ASSOCIATION_POINTS, "pressure");
  BridgeInternals::GlobalAnalysisAdaptor->Execute(BridgeInternals::GlobalDataAdaptor);
}

void senseireleasedata()
{
  BridgeInternals::GlobalDataAdaptor->ReleaseData();
}

void senseifinalize()
{
  cerr << "SENSEI: finalizing\n";
  BridgeInternals::GlobalDataAdaptor->ReleaseData();
  cerr << BridgeInternals::GlobalAnalysisAdaptor  << " SENSEI: finalizing   11111\n";
  BridgeInternals::GlobalAnalysisAdaptor = NULL;
  cerr << "SENSEI: finalizing  22222\n";
  BridgeInternals::GlobalDataAdaptor = NULL;
  cerr << "SENSEI: done finalizing\n";
}
