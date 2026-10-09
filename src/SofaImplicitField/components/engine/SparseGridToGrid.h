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
#pragma once
#include <SofaImplicitField/config.h>
#include <SofaImplicitField/components/geometry/DiscreteGridField.h>
#include <SofaImplicitField/components/loader/GridVDBLoader.h>
#include <sofa/core/topology/BaseMeshTopology.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace sofaimplicitfield::component::engine
{
using namespace sofa;

typedef sofa::core::topology::BaseMeshTopology::SeqTriangles SeqTriangles;
typedef sofa::core::topology::BaseMeshTopology::Triangle Triangle;
typedef sofa::type::vector<sofa::type::Vec3d> VecCoord;

using sofa::core::objectmodel::BaseComponent;
using sofa::component::geometry::ScalarField;
using sofa::core::visual::VisualParams ;
using sofa::type::Vec3d;
using sofa::type::Vec3u;


class SparseGridToGrid : public BaseComponent
{
public:
    SOFA_CLASS(SparseGridToGrid, BaseComponent);

    virtual void init() override ;
protected:
    Data<Vec3u> d_resolution;
    Data<Vec3d> d_min;
    Data<Vec3d> d_max;
    Data<bool>          d_debugDraw;

    Data<openvdb::FloatGrid::Ptr> d_source;
    Data<sofa::component::geometry::MemoryBuffer> d_buffer;

    SparseGridToGrid() ;
    virtual ~SparseGridToGrid() ;

    void checkInputs();
    void updateInternalBuffer(const Vec3u& resolution, const Vec3d& min, const Vec3d& max);
    void updateGridIfNeeded();
};

}

