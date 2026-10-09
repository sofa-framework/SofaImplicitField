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

#include "SparseGridToGrid.h"
#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>

namespace sofaimplicitfield::component::engine
{

SparseGridToGrid::SparseGridToGrid()
    : d_source(initData(&d_source, "source", "The VDB Sparse Grid to create a dense grid from."))
    , d_buffer(initData(&d_buffer, "buffer", "The destination buffer where the dense grid is stored."))
    , d_resolution(initData(&d_resolution, Vec3u(10, 10, 10), "resolution", "Number of samples in each dimension"))
    , d_min(initData(&d_min, Vec3d(-1.0, -1.0, -1.0), "min", "Minimum corner of the sampling grid"))
    , d_max(initData(&d_max, Vec3d(1.0, 1.0, 1.0), "max", "Maximum corner of the sampling grid"))
    , d_debugDraw(initData(&d_debugDraw,false, "debugDraw","Display the extracted surface"))
{
}

SparseGridToGrid::~SparseGridToGrid()
{
}

void SparseGridToGrid::updateInternalBuffer(const Vec3u& resolution, const Vec3d& min, const Vec3d& max)
{
    auto buffer = sofa::helper::getWriteOnlyAccessor(d_buffer);
    buffer->resize(min, max, resolution);
}

void SparseGridToGrid::init()
{
    if(!d_source.getValue().get())
    {
        msg_error() << "Missing field to extract surface from";
        d_componentState = core::objectmodel::ComponentState::Invalid;
    }

    checkInputs();
    updateInternalBuffer(d_resolution.getValue(), d_min.getValue(), d_max.getValue());
    updateGridIfNeeded();
    d_componentState = core::objectmodel::ComponentState::Valid;
}

void SparseGridToGrid::checkInputs()
{
}

void SparseGridToGrid::updateGridIfNeeded()
{
    auto sparsegrid = d_source.getValue();
    if(!sparsegrid)
        return;

    auto buffer = sofa::helper::getWriteAccessor(d_buffer);
    auto resolution = buffer->resolution;
    auto min = buffer->min;
    auto max = buffer->max;

    if (resolution[0] <= 0 || resolution[1] <= 0 || resolution[2] <= 0)
    {
        msg_error() << "Resolution must be greater than 0 in all dimensions";
        return;
    }

    if (min[0] >= max[0] || min[1] >= max[1] || min[2] >= max[2])
    {
        msg_error() << "min must be less than max in all dimensions";
        return;
    }

    std::cout << getPathName() << " sampling the grid " << std::endl;
    auto data = buffer->data;
    auto spacing = buffer->spacing;

    std::cout << "          : " << buffer->data << std::endl;

    int rx = resolution.x();
    int rxy = resolution.y() * rx;

    auto sampler = openvdb::tools::GridSampler<openvdb::FloatGrid, openvdb::tools::BoxSampler>{*(d_source.getValue())};

    for (unsigned int z = 0; z < resolution[2]; z++)
    {
        for (unsigned int y = 0; y < resolution[1]; y++)
        {
            for (unsigned int x = 0; x < resolution[0]; x++)
            {
                Vec3d p = min + Vec3d(static_cast<double>(x) * spacing[0],
                                            static_cast<double>(y) * spacing[1],
                                            static_cast<double>(z) * spacing[2]);
                auto index=[rxy,rx](int x,int y, int z){ return z * rxy + y * rx + x; };
                data[index(x,y,z)] = sampler.wsSample(openvdb::Vec3R(p.x(), p.y(), p.z()));
            }
        }
    }

    return;
}

// Register in the Factory
void registerSparseGridToGrid(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Generates a surface mesh from a field function.")
                             .add< SparseGridToGrid >());
}

}
