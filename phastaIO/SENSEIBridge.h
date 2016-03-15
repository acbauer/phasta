#ifndef phasta_sensei_bridge_h
#define phasta_sensei_bridge_h

extern "C" void senseiinitialize();
extern "C" void senseiinitializegrid(int* numPoints, double* coordsArray, int* numCells);
extern "C" void senseiaddcellblockinformation(int* numCellsInBlock, int* numPointsPerCell, int* cellConnectivity);
extern "C" void senseiaddfieldinformation(int* nshg, double* dofArray, int* compressibleFlow);
extern "C" void senseiupdate(int* tstep, double* time);
extern "C" void senseireleasedata();
extern "C" void senseifinalize();


#endif
