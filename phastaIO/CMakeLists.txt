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

if(USE_CATALYST)
  list(APPEND SRC_FILES CatalystAdaptor.cc)

  # set the Catalyst dependencies. it will be ok if it's
  # empty in target_link_libraries
  set(CATALYST_DEPENDENCIES vtkPVPythonCatalyst)

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
    senseiiinitialize senseiinitializegrid senseiaddcbi
    senseiaddfieldinformation senseiupdate senseireleasedata senseifinalize)
  include_directories(${phastaIO_BINARY_DIR})
endif()

ADD_LIBRARY(phastaIO ${SRC_FILES})
message("the libs are ${MPI_C_LIBRARIES} CC ${CATALYST_DEPENDENCIES} SS ${SENSEI_LIBS}")
target_link_libraries(phastaIO ${MPI_C_LIBRARIES} ${CATALYST_DEPENDENCIES} ${SENSEI_LIBS} vtkCommonDataModel)
