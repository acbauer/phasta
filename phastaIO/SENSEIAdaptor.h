#ifndef phasta_sensei_DataAdaptor_h
#define phasta_sensei_DataAdaptor_h

#include "DataAdaptor.h"
#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include <vector>

namespace sensei
{

/// @class SENSEIAdaptor
/// @brief SENSEIAdaptor is an abstract base class that defines the data interface.
///
/// SENSEIAdaptor defines the data interface for PHASTA. Analysis routines
/// (via AnalysisAdaptor) use this implementation to access simulation data.
class SENSEIAdaptor : public DataAdaptor
{
public:
  static SENSEIAdaptor* New();
  vtkTypeMacro(SENSEIAdaptor, DataAdaptor);

  /// @brief Return the data object with appropriate structure.
  ///
  /// This method will return a data object of the appropriate type. The data
  /// object can be a vtkDataSet subclass or a vtkCompositeDataSet subclass.
  /// If \c structure_only is set to true, then the geometry and topology
  /// information will not be populated. For data adaptors that produce a
  /// vtkCompositeDataSet subclass, passing \c structure_only will still produce
  /// appropriate composite data hierarchy.
  ///
  /// @param structure_only When set to true (default; false) the returned mesh
  /// may not have any geometry or topology information.
  virtual vtkDataObject* GetMesh(bool structure_only=false);

  /// @brief Adds the specified field array to the mesh.
  ///
  /// This method will add the requested array to the mesh, if available. If the
  /// array was already added to the mesh, this will not add it again. The mesh
  /// should not be expected to have geometry or topology information.
  ///
  /// @param association field association; one of
  /// vtkDataObject::FieldAssociations or vtkDataObject::AttributeTypes.
  /// @return true if array was added (or already added), false is request array
  /// is not available.
  virtual bool AddArray(vtkDataObject* mesh, int association, const char* arrayname);

  /// @brief Return the number of field arrays available.
  ///
  /// This method will return the number of field arrays available. For data
  /// adaptors producing composite datasets, this is a union of arrays available
  /// on all parts of the composite dataset.
  ///
  /// @param association field association; one of
  /// vtkDataObject::FieldAssociations or vtkDataObject::AttributeTypes.
  /// @return the number of arrays.
  virtual unsigned int GetNumberOfArrays(int association);
  /// @brief Return the name for a field array.
  ///
  /// This method will return the name for a field array given its index.
  ///
  /// @param association field association; one of
  /// vtkDataObject::FieldAssociations or vtkDataObject::AttributeTypes.
  /// @param index index for the array. Must be less than value returned
  /// GetNumberOfArrays().
  /// @return name of the array.
  virtual const char* GetArrayName(int association, unsigned int index);

  void InitializeGrid(int numPoints, double* coordsArray, int numCells);
  // just pass in the information needed to generate a block of cells
  void AddCellBlockInformation(int numCellsInBlock, int numPointsPerCell, int* cellConnectivity);
  // just pass in information for generating a field
  void AddFieldInformation(int nshg, double* dofArray, bool compressibleFlow);

  /// @brief Release data allocated for the current timestep.
  ///
  /// Releases the data allocated for the current timestep. This is expected to
  /// be called after each time iteration.
  virtual void ReleaseData();

protected:
  SENSEIAdaptor();
  virtual ~SENSEIAdaptor();

  int NumPoints;
  double* CoordsArray;
  int NumCells;

  std::vector<int> NumCellsInBlock;
  std::vector<int> NumPointsPerCell;
  std::vector<int*> CellConnectivity;

  int NSHG;
  double* DofArray;
  bool CompressibleFlow;

  vtkSmartPointer<vtkUnstructuredGrid> Mesh;

private:
  SENSEIAdaptor(const SENSEIAdaptor&); // Not implemented.
  void operator=(const SENSEIAdaptor&); // Not implemented.
};

}
#endif
