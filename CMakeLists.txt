cmake_minimum_required(VERSION 3.22)
project(SofaImplicitField VERSION 2.0)

if(TARGET blosc)
    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        sofa_install_libraries(TARGETS blosc)
    endif()
elseif( (DEFINED SOFA_ALLOW_FETCH_DEPENDENCIES AND SOFA_ALLOW_FETCH_DEPENDENCIES) OR (NOT DEFINED SOFA_ALLOW_FETCH_DEPENDENCIES))
    message("${PROJECT_NAME}: blosc not found and SOFA_ALLOW_FETCH_DEPENDENCIES is ON, fetching source code...")

    FetchContent_Declare(
        blosc
        GIT_REPOSITORY https://github.com/Blosc/c-blosc.git
        GIT_TAG v1.21.6
    )

    # 1. Fetch Blosc
    FetchContent_Populate(blosc)

    # 2. add_subdirectory(Blosc)
    add_subdirectory(
        "${blosc_SOURCE_DIR}"
        "${blosc_BINARY_DIR}"
    )

    # 3. Définir ce qu'OpenVDB doit utiliser
    set(Blosc_INCLUDE_DIR
        "${blosc_SOURCE_DIR}/include"
        CACHE PATH "" FORCE
    )

    set(Blosc_LIBRARY
        "${blosc_BINARY_DIR}/lib/libblosc.a"
        CACHE FILEPATH "" FORCE
    )

endif()

if(TARGET openvdb)
    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        sofa_install_libraries(TARGETS openvdb)
    endif()
elseif( (DEFINED SOFA_ALLOW_FETCH_DEPENDENCIES AND SOFA_ALLOW_FETCH_DEPENDENCIES) OR (NOT DEFINED SOFA_ALLOW_FETCH_DEPENDENCIES))
    # Download if
    # 1) SOFA_ALLOW_FETCH_DEPENDENCIES is defined, meaning a in-tree project AND its value is TRUE
    # 2) SOFA_ALLOW_FETCH_DEPENDENCIES is not defined, which means it is a out-of-tree project
    message("${PROJECT_NAME}: openvdb not found and SOFA_ALLOW_FETCH_DEPENDENCIES is ON, fetching source code...")
    # OpenVDB options
    set(OPENVDB_BUILD_BINARIES OFF CACHE BOOL "" FORCE)
    set(OPENVDB_BUILD_UNITTESTS OFF CACHE BOOL "" FORCE)
    set(OPENVDB_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(OPENVDB_BUILD_NANOVDB OFF CACHE BOOL "" FORCE)

    # NanoVDB dependencies
    set(NANOVDB_USE_TBB OFF CACHE BOOL "" FORCE)
    set(NANOVDB_USE_BLOSC OFF CACHE BOOL "" FORCE)
    set(NANOVDB_USE_ZLIB OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        openvdb
        GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openvdb.git
        GIT_TAG v13.1.0
    )

    FetchContent_GetProperties(openvdb)
    if(NOT openvdb_POPULATED)
        FetchContent_Populate(openvdb)

        # OpenVDB 13.1.0 utilise CMAKE_SOURCE_DIR pour trouver
        # OpenVDBUtils.cmake, ce qui ne fonctionne pas lorsqu'il est
        # inclus comme sous-projet.
        file(READ
            "${openvdb_SOURCE_DIR}/openvdb/openvdb/CMakeLists.txt"
            OPENVDB_CMAKELISTS
        )

        string(REPLACE
            "include(\${CMAKE_SOURCE_DIR}/cmake/OpenVDBUtils.cmake)"
            "include(\${CMAKE_SOURCE_DIR}/cmake/OpenVDBUtils.cmake)"
            OPENVDB_CMAKELISTS
            "${OPENVDB_CMAKELISTS}"
        )

        file(WRITE
            "${openvdb_SOURCE_DIR}/openvdb/openvdb/CMakeLists.txt"
            "${OPENVDB_CMAKELISTS}"
        )
    endif()

    file(
        READ
        "${openvdb_SOURCE_DIR}/CMakeLists.txt"
        OPENVDB_ROOT_CMAKELISTS
    )

    string(
        REPLACE
        "add_custom_target(uninstall"
        "add_custom_target(openvdb_uninstall"
        OPENVDB_ROOT_CMAKELISTS
        "${OPENVDB_ROOT_CMAKELISTS}"
    )

    file(
        WRITE
        "${openvdb_SOURCE_DIR}/CMakeLists.txt"
        "${OPENVDB_ROOT_CMAKELISTS}"
    )



    #FetchContent_MakeAvailable(openvdb)

    add_subdirectory(
        "${openvdb_SOURCE_DIR}"
        "${openvdb_BINARY_DIR}"
    )

    get_target_property(OPENVDB_TYPE openvdb TYPE)
    get_target_property(OPENVDB_IMPORTED openvdb IMPORTED)

    message(STATUS "OpenVDB TYPE     = ${OPENVDB_TYPE}")
    message(STATUS "OpenVDB IMPORTED = ${OPENVDB_IMPORTED}")

endif()

if(TARGET nanovdb)
    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        sofa_install_libraries(TARGETS nanovdb)
    endif()
elseif( (DEFINED SOFA_ALLOW_FETCH_DEPENDENCIES AND SOFA_ALLOW_FETCH_DEPENDENCIES) OR (NOT DEFINED SOFA_ALLOW_FETCH_DEPENDENCIES))
    # Download if
    # 1) SOFA_ALLOW_FETCH_DEPENDENCIES is defined, meaning a in-tree project AND its value is TRUE
    # 2) SOFA_ALLOW_FETCH_DEPENDENCIES is not defined, which means it is a out-of-tree project
    message("${PROJECT_NAME}: nanovdb not found and SOFA_ALLOW_FETCH_DEPENDENCIES is ON, fetching source code...")
    #set(nanovdb OFF CACHE INTERNAL "Build the nanovbd example programs")
    #set(nanovdb_BUILD_TESTS OFF CACHE INTERNAL "Build the openvdb test programs")
    set(NANOVDB_USE_TBB OFF CACHE INTERNAL "")
    set(NANOVDB_USE_BLOSC OFF CACHE INTERNAL "")
    set(NANOVDB_USE_ZLIB OFF CACHE INTERNAL "")

    FetchContent_Declare(
        nanovdb
        GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openvdb.git
        GIT_TAG v13.1.0
    )

    FetchContent_GetProperties(nanovdb)

    if(NOT nanovdb_POPULATED)
        FetchContent_Populate(nanovdb)
    endif()
endif()

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
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/BottleField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/DiscreteGridField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/SphericalField.h
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/ScalarField.h
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
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/BottleField.cpp
    ${SOFAIMPLICITFIELD_SRC}/components/geometry/ScalarField.cpp
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
target_include_directories(${PROJECT_NAME} PRIVATE ${nanovdb_SOURCE_DIR}/nanovdb)

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
