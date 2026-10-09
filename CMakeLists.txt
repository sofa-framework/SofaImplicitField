cmake_minimum_required(VERSION 3.22)
project(SofaImplicitField VERSION 2.0)

#find_package(OpenVDB REQUIRED)

add_library(blosc STATIC IMPORTED)
set_target_properties(blosc PROPERTIES
    IMPORTED_LOCATION "/usr/local/lib/libblosc.so"
    INTERFACE_INCLUDE_DIRECTORIES "/usr/local/include"
)
add_library(openvdb STATIC IMPORTED)
set_target_properties(openvdb PROPERTIES
    IMPORTED_LOCATION "/usr/local/lib/libopenvdb.so"
    INTERFACE_INCLUDE_DIRECTORIES "/usr/local/include"
)

if(NOT TARGET Sofa.Framework)
    find_package(Sofa.Framework REQUIRED) #For macros
endif()
sofa_find_package(Sofa.Component.Topology.Container.Constant REQUIRED)

set(SOFAIMPLICITFIELD_SRC "src/${PROJECT_NAME}")

set(HEADER_FILES
    ${SOFAIMPLICITFIELD_SRC}/config.h.in
    ${SOFAIMPLICITFIELD_SRC}/initSofaImplicitField.h
    ${SOFAIMPLICITFIELD_SRC}/MarchingCube.h
    ${SOFAIMPLICITFIELD_SRC}/MHD.h

    ${SOFAIMPLICITFIELD_SRC}/components/engine/FieldToGaussianSplat.h
    ${SOFAIMPLICITFIELD_SRC}/components/engine/FieldToSurfaceMesh.h
    ${SOFAIMPLICITFIELD_SRC}/components/engine/GridSampler.h
    ${SOFAIMPLICITFIELD_SRC}/components/engine/SparseGridToGrid.h
    ${SOFAIMPLICITFIELD_SRC}/components/engine/SparseGridToSurfaceMesh.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/BottleField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/DiscreteGridField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/SphericalField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/ScalarField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/SparseGridField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/StarShapedField.h
    ${SOFAIMPLICITFIELD_SRC}/components/loader/MHDLoader.h
    ${SOFAIMPLICITFIELD_SRC}/components/loader/GridVDBLoader.h
    ${SOFAIMPLICITFIELD_SRC}/components/loader/MHDWriter.h
    ${SOFAIMPLICITFIELD_SRC}/components/mapping/ImplicitSurfaceMapping.h
    ${SOFAIMPLICITFIELD_SRC}/components/mapping/ImplicitSurfaceMapping.inl
    ${SOFAIMPLICITFIELD_SRC}/components/mapping/ScalarFieldMapping.h
)

set(SOURCE_FILES
    ${SOFAIMPLICITFIELD_SRC}/initSofaImplicitField.cpp
    ${SOFAIMPLICITFIELD_SRC}/MarchingCube.cpp
    ${SOFAIMPLICITFIELD_SRC}/MHD.cpp

    ${SOFAIMPLICITFIELD_SRC}/components/engine/FieldToGaussianSplat.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/engine/FieldToSurfaceMesh.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/engine/GridSampler.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/engine/SparseGridToGrid.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/engine/SparseGridToSurfaceMesh.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/BottleField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/ScalarField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/SparseGridField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/DiscreteGridField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/SphericalField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/StarShapedField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/loader/MHDLoader.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/loader/GridVDBLoader.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/loader/MHDWriter.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/mapping/ImplicitSurfaceMapping.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/mapping/ScalarFieldMapping.cpp
)

set(EXTRA_FILES
    README.md
    )

if(SOFA_BUILD_TESTS)
    add_subdirectory(SofaImplicitField_test)
endif()

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${EXTRA_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE Sofa.Component.Topology.Container.Constant)
target_link_libraries(${PROJECT_NAME} PRIVATE openvdb)

find_package(SofaPython3 REQUIRED)
if (SofaPython3_FOUND)
  add_subdirectory(python)
endif()

## Install rules for the library and headers; CMake package configurations files
sofa_create_package_with_targets(
    PACKAGE_NAME ${PROJECT_NAME}
    PACKAGE_VERSION ${PROJECT_VERSION}
    TARGETS ${PROJECT_NAME} AUTO_SET_TARGET_PROPERTIES
    INCLUDE_SOURCE_DIR "src"
    INCLUDE_INSTALL_DIR "${PROJECT_NAME}"
    RELOCATABLE "plugins"
    )
