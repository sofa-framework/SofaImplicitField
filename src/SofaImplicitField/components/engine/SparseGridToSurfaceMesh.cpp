/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2025 INRIA, USTL, UJF, CNRS, MGH                    *
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

#include <sofa/core/visual/VisualParams.h>
using sofa::core::visual::VisualParams ;

#include <sofa/core/ObjectFactory.h>
using sofa::core::RegisterObject ;

#include <SofaImplicitField/components/engine/SparseGridToSurfaceMesh.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/tools/VolumeToMesh.h>

namespace sofaimplicitfield::component::engine
{

SparseGridToSurfaceMesh::SparseGridToSurfaceMesh()
    : d_source(initData(&d_source, "source", "The VDB Sparse Grid to create a dense grid from."))
    , d_outPoints(initData(&d_outPoints, "points", "position of the tiangles vertex"))
    , d_outTriangles(initData(&d_outTriangles, "triangles", "list of triangles"))
    , d_outQuads(initData(&d_outQuads, "quads", "list of quads"))
    , d_debugDraw(initData(&d_debugDraw,false, "debugDraw","Display the extracted surface"))
{
}

SparseGridToSurfaceMesh::~SparseGridToSurfaceMesh()
{
}

void SparseGridToSurfaceMesh::init()
{
    if(!d_source.getValue().get())
    {
        msg_error() << "Missing field to extract surface from";
        d_componentState = core::objectmodel::ComponentState::Invalid;
    }

    updateIfNeeded();
    d_componentState = core::objectmodel::ComponentState::Valid;
}

void SparseGridToSurfaceMesh::updateIfNeeded()
{
    std::cout << "STEP 1 " << std::endl;
    auto sparsegrid = d_source.getValue();
    if(!sparsegrid)
        return;
    std::cout << "STEP 2 " << std::endl;

    auto points = sofa::helper::getWriteOnlyAccessor(d_outPoints);
    auto triangles = sofa::helper::getWriteOnlyAccessor(d_outTriangles);
    auto quads = sofa::helper::getWriteOnlyAccessor(d_outQuads);

    std::vector<openvdb::Vec3s> spoints;
    std::vector<openvdb::Vec3I> striangles;
    std::vector<openvdb::Vec4I> squads;
    openvdb::tools::volumeToMesh(*sparsegrid, spoints, striangles, squads, /*isovalue=*/0.0, /*adaptivity=*/1.0);

    for(auto pt : spoints)
        points.push_back(Vec3d(pt[0], pt[1], pt[2]));

    for(auto tri : striangles)
        triangles.push_back(Triangle(tri[0], tri[1], tri[2]));

    for(auto quad : squads)
        quads.push_back(Quad{quad[0], quad[1], quad[2], quad[3]});

    msg_warning() << "Surface generated: " << spoints.size() << " pts, "
                                           << striangles.size() << " tris, "
                                           << squads.size() << " quads";
    return;
}

// Register in the Factory
void registerSparseGridToSurfaceMesh(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Generates a surface mesh from a field function.")
                             .add< SparseGridToSurfaceMesh >());
}

}
