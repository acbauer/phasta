#include "SENSEIBridgeAPIMangling.h"

#include "SENSEIBridge.h"
#include "SENSEIAdaptor.h"
#include <SliceExtract.h>
#include <vtkMPIController.h>

#include <timer/Timer.h>

namespace BridgeInternals
{
  static vtkSmartPointer<vtkMPIController> Controller;
  static vtkSmartPointer<sensei::SENSEIAdaptor> GlobalDataAdaptor;
  static vtkSmartPointer<sensei::SliceExtract> AnalysisAdaptor;

  // for timing
  timer::MarkEvent* Timer = NULL;
  int Counter = 0;
}

void senseiinitialize()
{
  timer::MarkEvent mark("senseiinitialize");
  if (!BridgeInternals::Controller)
    {
    BridgeInternals::Controller = vtkSmartPointer<vtkMPIController>::New();
    BridgeInternals::Controller->Initialize(0, 0, 1);
    BridgeInternals::Controller->SetGlobalController(BridgeInternals::Controller);
    }
  if (!BridgeInternals::GlobalDataAdaptor)
    {
    BridgeInternals::GlobalDataAdaptor = vtkSmartPointer<sensei::SENSEIAdaptor>::New();
    }
  if (!BridgeInternals::AnalysisAdaptor)
    {
    BridgeInternals::AnalysisAdaptor = vtkSmartPointer<sensei::SliceExtract>::New();
    }
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
  BridgeInternals::AnalysisAdaptor->Execute(BridgeInternals::GlobalDataAdaptor);
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
  timer::MarkStartEvent("senseifinalize");
  BridgeInternals::GlobalDataAdaptor->ReleaseData();
  BridgeInternals::AnalysisAdaptor = NULL;
  BridgeInternals::GlobalDataAdaptor = NULL;
  BridgeInternals::Controller->Finalize(1);
  BridgeInternals::Controller->SetGlobalController(NULL);
  BridgeInternals::Controller = NULL;
  timer::MarkEndEvent("senseifinalize");
  timer::PrintLog(std::cerr, MPI_COMM_WORLD);
}
