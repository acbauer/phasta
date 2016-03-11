/*=========================================================================

  Program:   ParaView
  Module:    SENSEIAdaptor.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "SENSEIAdaptor.h"

#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSOADataArrayTemplate.h" // for VTK v. 7.1+
#include "vtkUnstructuredGrid.h"

namespace
{
void InsertBlockOfCells(
    vtkUnstructuredGrid* mesh, int numCellsInBlock,
    int numPointsPerCell, int* cellConnectivity)
{
  int type = -1;
  switch(numPointsPerCell)
    {
    case 4:
      {
      type = VTK_TETRA;
      break;
      }
    case 5:
      {
      type = VTK_PYRAMID;
      break;
      }
    case 6:
      {
      type = VTK_WEDGE;
      break;
      }
    case 8:
      {
      type = VTK_HEXAHEDRON;
      break;
      }
    default:
      {
      vtkGenericWarningMacro("SENSEI: Incorrect amount of vertices per element: "
                             << numPointsPerCell);
      return;
      }
    }
  vtkIdType pts[8]; //assume for now we only have linear elements
  vtkIdType numPoints = mesh->GetNumberOfPoints();
  for(int iCell=0;iCell<numCellsInBlock;iCell++)
    {
    for(int i=0;i<numPointsPerCell;i++)
      {
      pts[i] = cellConnectivity[iCell+i*numCellsInBlock]-1;//-1 to get from f to c++

      if(pts[i] < 0 || pts[i] >= numPoints)
        {
        vtkGenericWarningMacro(<<pts[i] << " is not a valid node id.");
        }
      }
    if(type == VTK_TETRA)
      { // change the canonical ordering of the tet to match VTK style
      vtkIdType temp = pts[0];
      pts[0] = pts[1];
      pts[1] = temp;
      }
    mesh->InsertNextCell(type, numPointsPerCell, pts);
    }
}
} // end anonymous namespace

namespace sensei
{

vtkStandardNewMacro(SENSEIAdaptor);
// arbitrarily setting CompressibleFlow to false just to have a value set
//-----------------------------------------------------------------------------
// SENSEIAdaptor::SENSEIAdaptor() : NumPoints(0), CoordsArray(NULL), NumCells(0),
//                                  NSHG(0), DofArray(NULL), CompressibleFlow(false)
// {
// }
SENSEIAdaptor::SENSEIAdaptor()
{
  this->NumPoints = 0;
  this->CoordsArray = NULL;
  this->NumCells = 0;
  this->NSHG = 0;
  this->DofArray = NULL;
  this->CompressibleFlow = false;
}

//-----------------------------------------------------------------------------
SENSEIAdaptor::~SENSEIAdaptor()
{
}

//-----------------------------------------------------------------------------
void SENSEIAdaptor::InitializeGrid(
  int numPoints, double* coordsArray, int numCells)
{
  cerr << this << " ACB numpoints " << numPoints << " CA " << coordsArray << " NC " << numCells << endl;

  this->NumPoints = numPoints;
  this->CoordsArray = coordsArray;
  this->NumCells = numCells;
  cerr << "DONE ACB numpoints " << numPoints << " CA " << coordsArray << " NC " << numCells << endl;
}

//-----------------------------------------------------------------------------
void SENSEIAdaptor::AddCellBlockInformation(
  int numCellsInBlock, int numPointsPerCell, int* cellConnectivity)
{
  this->NumCellsInBlock.push_back(numCellsInBlock);
  this->NumPointsPerCell.push_back(numPointsPerCell);
  this->CellConnectivity.push_back(cellConnectivity);
}

//-----------------------------------------------------------------------------
void SENSEIAdaptor::AddFieldInformation(
  int nshg, double* dofArray, bool compressibleFlow)
{
  this->NSHG = nshg;
  this->DofArray = dofArray;
  this->CompressibleFlow = compressibleFlow;
}

//-----------------------------------------------------------------------------
void SENSEIAdaptor::ReleaseData()
{
  this->NumPoints = 0;
  this->CoordsArray = NULL;
  this->NumCells = 0;
  this->NumCellsInBlock.clear();
  this->NumPointsPerCell.clear();
  this->CellConnectivity.clear();
  this->NSHG = 0;
  this->DofArray = NULL;
  this->Mesh = NULL;
}

//-----------------------------------------------------------------------------
vtkDataObject* SENSEIAdaptor::GetMesh(bool vtkNotUsed(structure_only))
{
  if(!this->Mesh)
    {
    this->Mesh = vtkSmartPointer<vtkUnstructuredGrid>::New();
    vtkPoints* nodePoints = vtkPoints::New();
    // pre VTK v. 7.1
    // vtkDoubleArray* coords = vtkDoubleArray::New();
    // coords->SetNumberOfComponents(3);
    // coords->SetNumberOfTuples(this->NumPoints);
    // for(int i=0;i<this->NumPoints;i++)
    //   {
    //   double tuple[3] = {this->CoordsArray[i], this->CoordsArray[i+this->NumPoints],
    //                      this->CoordsArray[i+this->NumPoints*2]};
    //   coords->SetTupleValue(i, tuple);
    //   }
    vtkSOADataArrayTemplate<double>* coords = vtkSOADataArrayTemplate<double>::New();
    coords->SetNumberOfComponents(3);
    coords->SetNumberOfTuples(this->NumPoints);
    coords->SetArray(0, this->CoordsArray, this->NumPoints, false, true);
    coords->SetArray(1, this->CoordsArray+this->NumPoints, this->NumPoints, false, true);
    coords->SetArray(2, this->CoordsArray+this->NumPoints*2, this->NumPoints, false, true);
    nodePoints->SetData(coords);
    coords->Delete();
    this->Mesh->SetPoints(nodePoints);
    nodePoints->Delete();
    this->Mesh->Allocate(this->NumCells);
    for(size_t i=0;i<this->NumCellsInBlock.size();i++)
      {
      InsertBlockOfCells(this->Mesh, this->NumCellsInBlock[i],
                         this->NumPointsPerCell[i], this->CellConnectivity[i]);
      }
    }
  return this->Mesh;
}

//-----------------------------------------------------------------------------
  bool SENSEIAdaptor::AddArray(vtkDataObject* mesh, int association,
                               const std::string& arrayname)
{
  if (association == vtkDataObject::FIELD_ASSOCIATION_CELLS)
    {
    return false;
    }

  vtkDataSet* dataSet = vtkDataSet::SafeDownCast(mesh);

  if(arrayname == "velocity")
    {
    // pre VTK v. 7.1
    // vtkDoubleArray* velocity = vtkDoubleArray::New();
    // velocity->SetName("velocity");
    // velocity->SetNumberOfComponents(3);
    // velocity->SetNumberOfTuples(dataSet->GetNumberOfPoints());
    // for (vtkIdType idx=0; idx<dataSet->GetNumberOfPoints(); idx++)
    //   {
    //   velocity->SetTuple3(idx, this->DofArray[idx],
    //                       this->DofArray[idx+this->NSHG],
    //                       this->DofArray[idx+this->NSHG*2]);
    //   }
    // dataSet->GetPointData()->AddArray(velocity);
    // velocity->Delete();
    // return true;

    vtkSOADataArrayTemplate<double>* velocity = vtkSOADataArrayTemplate<double>::New();
    velocity->SetName("velocity");
    velocity->SetNumberOfComponents(3);
    velocity->SetNumberOfTuples(dataSet->GetNumberOfPoints());
    velocity->SetArray(0, this->DofArray, dataSet->GetNumberOfPoints(), false, true);
    velocity->SetArray(1, this->DofArray+this->NSHG, dataSet->GetNumberOfPoints(), false, true);
    velocity->SetArray(2, this->DofArray+this->NSHG*2, dataSet->GetNumberOfPoints(), false, true);
    dataSet->GetPointData()->AddArray(velocity);
    velocity->Delete();
    return true;

    }
  if(arrayname == "pressure")
    {
    vtkDoubleArray* pressure = vtkDoubleArray::New();
    pressure->SetName("pressure");
    pressure->SetArray(this->DofArray+this->NSHG*3, dataSet->GetNumberOfPoints(), 1);
    dataSet->GetPointData()->AddArray(pressure);
    pressure->Delete();
    return true;
    }

  // temperature only varies from compressible flow
  if(arrayname == "temperature" && this->CompressibleFlow == 1)
    {
    vtkDoubleArray* temperature = vtkDoubleArray::New();
    temperature->SetName("temperature");
    temperature->SetArray(this->DofArray+this->NSHG*4,
                          dataSet->GetNumberOfPoints(), 1);
    dataSet->GetPointData()->AddArray(temperature);
    temperature->Delete();
    return true;
    }

  return false;
}

unsigned int SENSEIAdaptor::GetNumberOfArrays(int association)
{
  if (association != vtkDataObject::FIELD_ASSOCIATION_CELLS)
    {
    return 0;
    }
  else if(this->CompressibleFlow == 1)
    {
    return 3;
    }
  return 2;
}

std::string SENSEIAdaptor::GetArrayName(int association, unsigned int index)
{
  if (association != vtkDataObject::FIELD_ASSOCIATION_CELLS)
    {
    return NULL;
    }
  if(index == 0)
    {
    return "velocity";
    }
  if(index == 1)
    {
    return "pressure";
    }
  else if(this->CompressibleFlow == 1 && index == 2)
    {
    return "temperature";
    }
  return NULL;
}

} // end sensei namespace
