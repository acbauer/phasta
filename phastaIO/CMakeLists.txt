project(phastaIO)
enable_language(CXX)

find_package(MPI REQUIRED)
include_directories(${MPI_C_INCLUDE_PATH})
# collect all the source files
include(FortranCInterface)
FortranCInterface_VERIFY(CXX)
FortranCInterface_Header(FCMangle.h)
include_directories("${phastaIO_BINARY_DIR}")
add_definitions(-DMPICH_SKIP_MPICXX)
add_definitions(-DOMPI_SKIP_MPICXX=1)
set(SRC_FILES phiotmrc.cc phastaIO.cc)

set(LINK_LIBRARIES ${MPI_C_LIBRARIES})

if(USE_CATALYST)
  list(APPEND SRC_FILES CatalystAdaptor.cc)

  list(APPEND LINK_LIBRARIES vtkPVPythonCatalyst)

  # below is to have CMake mangle the C/C++ functions
  # to be callable from Fortran.
  include(FortranCInterface)
  FortranCInterface_HEADER(CatalystAdaptorAPIMangling.h SYMBOLS
    createpointsandallocatecells insertblockofcells addfields)
  include_directories(${phastaIO_BINARY_DIR})
endif()

if(USE_SENSEI)
  include_directories(${SENSEI_INCLUDE_DIRS})
  list(APPEND SRC_FILES SENSEIAdaptor.cc SENSEIBridge.cc)

  # below is to have CMake mangle the C/C++ functions
  # to be callable from Fortran.
  include(FortranCInterface)
  FortranCInterface_HEADER(SENSEIBridgeAPIMangling.h SYMBOLS
    senseiinitialize senseiinitializegrid senseiaddcbi
    senseiaddfieldinformation senseiupdate senseireleasedata senseifinalize)
  include_directories(${phastaIO_BINARY_DIR})

  list(APPEND LINK_LIBRARIES ${SENSEI_LIBS} vtkCommonDataModel ${MPI_CXX_LIBRARIES})
endif()

ADD_LIBRARY(phastaIO ${SRC_FILES})
#target_link_libraries(phastaIO ${MPI_C_LIBRARIES} ${CATALYST_DEPENDENCIES} ${SENSEI_LIBS} vtkCommonDataModel ${MPI_CXX_LIBRARIES})
target_link_libraries(phastaIO ${LINK_LIBRARIES})
