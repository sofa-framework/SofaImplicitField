/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <SofaImplicitField/config.h>
#include <SofaImplicitField/components/loader/GridVDBLoader.h>
#include <sofa/core/visual/VisualParams.h>

#include <sofa/core/ObjectFactory.h>
using sofa::core::RegisterObject ;

#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/io/io.h>

//#include <nanovdb/io/IO.h> // this is required to read (and write) NanoVDB files on the host

namespace sofaimplicitfield::component::io
{

GridVDBLoader::GridVDBLoader()
    : d_vdbgrid(initData(&d_vdbgrid, "vdbgrid", "The VDB Sparse Grid to create a dense grid from."))
{}

GridVDBLoader::~GridVDBLoader(){}

void GridVDBLoader::init()
{
    Inherit1::init();
    d_componentState = sofa::core::objectmodel::ComponentState::Valid;
}

// Populate the given grid with a narrow-band level set representation of a sphere.
// The width of the narrow band is determined by the grid's background value.
// (Example code only; use tools::createSphereSDF() in production.)
template<class GridType>
void
makeSphere(GridType& grid, float radius, const openvdb::Vec3f& c)
{
    using ValueT = typename GridType::ValueType;

    // Distance value for the constant region exterior to the narrow band
    const ValueT outside = grid.background();

    // Distance value for the constant region interior to the narrow band
    // (by convention, the signed distance is negative in the interior of
    // a level set)
    const ValueT inside = -outside;

    // Use the background value as the width in voxels of the narrow band.
    // (The narrow band is centered on the surface of the sphere, which
    // has distance 0.)
    int padding = int(openvdb::math::RoundUp(openvdb::math::Abs(outside)));
    // The bounding box of the narrow band is 2*dim voxels on a side.
    int dim = int(radius + padding);

    // Get a voxel accessor.
    typename GridType::Accessor accessor = grid.getAccessor();

    // Compute the signed distance from the surface of the sphere of each
    // voxel within the bounding box and insert the value into the grid
    // if it is smaller in magnitude than the background value.
    openvdb::Coord ijk;
    int &i = ijk[0], &j = ijk[1], &k = ijk[2];
    for (i = c[0] - dim; i < c[0] + dim; ++i) {
        const float x2 = openvdb::math::Pow2(i - c[0]);
        for (j = c[1] - dim; j < c[1] + dim; ++j) {
            const float x2y2 = openvdb::math::Pow2(j - c[1]) + x2;
            for (k = c[2] - dim; k < c[2] + dim; ++k) {

                // The distance from the sphere surface in voxels
                const float dist = openvdb::math::Sqrt(x2y2
                                                       + openvdb::math::Pow2(k - c[2])) - radius;

                // Convert the floating-point distance to the grid's value type.
                ValueT val = ValueT(dist);

                // Only insert distances that are smaller in magnitude than
                // the background value.
                if (val < inside || outside < val) continue;

                // Set the distance for voxel (i,j,k).
                accessor.setValue(ijk, val);
            }
        }
    }

    // Propagate the outside/inside sign information from the narrow band
    // throughout the grid.
    openvdb::tools::signedFloodFill(grid.tree());
}

bool GridVDBLoader::load()
{
    // -- Loading file
    const std::string filename = d_filename.getFullPath();

    dmsg_info() << "Loading VDH file: " << filename;

    // Initialize the OpenVDB library.  This must be called at least
    // once per program and may safely be called multiple times.
    openvdb::initialize();

    // Create a VDB file object.
    openvdb::io::File file(d_filename.getAbsolutePath());

    // Open the file.  This reads the file header, but not any grids.
    file.open();

    // Loop over all grids in the file and retrieve a shared pointer
    // to the one named "LevelSetSphere".  (This can also be done
    // more simply by calling file.readGrid("LevelSetSphere").)
    openvdb::GridBase::Ptr baseGrid;
    for (openvdb::io::File::NameIterator nameIter = file.beginName();
         nameIter != file.endName(); ++nameIter)
    {
        std::cout << "skipping grid '" << nameIter.gridName() <<"'" << std::endl;
        baseGrid = file.readGrid(nameIter.gridName());
    }

    // From the example above, "LevelSetSphere" is known to be a FloatGrid,
    // so cast the generic grid pointer to a FloatGrid pointer.
    openvdb::FloatGrid::Ptr grid2 = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
    grid2->setTransform(openvdb::math::Transform::createLinearTransform(1/75.0));

    file.close();

    d_vdbgrid.setValue(grid2);

    return true;
}

// Register in the Factory
void registerGridVDBLoader(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Load a VDB file storing a scalar field.")
    .add< GridVDBLoader >());
}

}

namespace sofa::core::objectmodel
{
/// Specialization for MemoryBuffer
template<> bool Data<openvdb::FloatGrid::Ptr>::read( const std::string&) { return false; }
template<> void Data<openvdb::FloatGrid::Ptr>::printValue( std::ostream& ) const {}
template<> std::string Data<openvdb::FloatGrid::Ptr>::getValueString() const {
    std::stringstream tmp;
    auto& buffer = getValue();
    tmp << "VDB Sparse Grid: ";
    if(buffer)
        buffer->print(tmp,1);
    else
        tmp << "null grid";
    return tmp.str() ; }
template<> std::string Data<openvdb::FloatGrid::Ptr>::getDefaultValueString() const { return ""; }
}
