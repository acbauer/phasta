#include "SENSEIBridgeAPIMangling.h"

#include "SENSEIBridge.h"
#include "SENSEIAdaptor.h"
#include <Autocorrelation.h>
#include <GeneralAutocorrelation.h>

#include <timer/Timer.h>

namespace BridgeInternals
{
  static vtkSmartPointer<sensei::SENSEIAdaptor> GlobalDataAdaptor;
  static vtkSmartPointer<sensei::GeneralAutocorrelation> GAAdaptor;
  static vtkSmartPointer<sensei::Autocorrelation> AAdaptor; // not currently working

  // for timing
  timer::MarkEvent* Timer = NULL;
  int Counter = 0;
}

void senseiinitialize()
{
  timer::MarkEvent mark("senseiinitialize");
  if (!BridgeInternals::GlobalDataAdaptor)
    {
    BridgeInternals::GlobalDataAdaptor = vtkSmartPointer<sensei::SENSEIAdaptor>::New();
    }
  if (!BridgeInternals::GAAdaptor)
    {
    BridgeInternals::GAAdaptor = vtkSmartPointer<sensei::GeneralAutocorrelation>::New();
    std::string arrayname = "pressure";
    BridgeInternals::GAAdaptor->Initialize(4, vtkDataObject::FIELD_ASSOCIATION_POINTS, arrayname);
    }
  // if (!BridgeInternals::AAdaptor)
  //   {
  //   BridgeInternals::AAdaptor = vtkSmartPointer<sensei::Autocorrelation>::New();
  //   std::string arrayname = "pressure";
  //   BridgeInternals::AAdaptor->Initialize(MPI_COMM_WORLD, 2, vtkDataObject::FIELD_ASSOCIATION_POINTS, arrayname, 5);
  //   }
}

void senseiinitializegrid(int* numPoints, double* coordsArray, int* numCells)
{
  if(!BridgeInternals::Timer)
    {
    timer::MarkStartTimeStep(BridgeInternals::Counter, BridgeInternals::Counter);
    BridgeInternals::Counter++;
    BridgeInternals::Timer = new timer::MarkEvent("SENSEICompute");
    }
  else
    {
    cerr << "Something wrong with the timer\n";
    }
  BridgeInternals::GlobalDataAdaptor->InitializeGrid(*numPoints, coordsArray, *numCells);
}

void senseiaddcellblockinformation(
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
  BridgeInternals::GAAdaptor->Execute(BridgeInternals::GlobalDataAdaptor);
  //BridgeInternals::AAdaptor->Execute(BridgeInternals::GlobalDataAdaptor);
}

void senseireleasedata()
{
  BridgeInternals::GlobalDataAdaptor->ReleaseData();
  if(BridgeInternals::Timer)
    {
    delete BridgeInternals::Timer;
    BridgeInternals::Timer = NULL;
    timer::MarkEndTimeStep();
    }
  else
    {
    cerr << "Something wrong with the timer (finalize)\n";
    }
}

void senseifinalize()
{
  timer::MarkEvent mark("senseifinalize");
  BridgeInternals::GlobalDataAdaptor->ReleaseData();
  BridgeInternals::GAAdaptor = NULL;
  BridgeInternals::AAdaptor = NULL;
  BridgeInternals::GlobalDataAdaptor = NULL;
}
