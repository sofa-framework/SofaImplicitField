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
#pragma once
#include <SofaImplicitField/config.h>

#include <sofa/core/objectmodel/DataFileName.h>
#include <SofaImplicitField/components/geometry/ScalarField.h>
#include <SofaImplicitField/components/loader/GridVDBLoader.h>

namespace sofa::component::geometry
{

namespace
{
using sofa::type::Vec3;
using sofa::type::Vec3u;
using sofa::type::Vec3d;
}



class SOFA_SOFAIMPLICITFIELD_API SparseGridField : public virtual ScalarField
{
public:
    SOFA_CLASS(SparseGridField, ScalarField);

    SparseGridField();
    ~SparseGridField() override;

    void init() override;
    void draw(const sofa::core::visual::VisualParams*) override;

    double getValue(const Vec3d& position, int& domain) override;
    void getValues(const std::vector<Vec3d>& positions, std::vector<double>& results) override;

    Vec3d getGradient(const Vec3d& position, int& domain) override;
    void getHessian(const Vec3d& positions, type::Mat3x3d& result) override;

    Data<bool> d_debugDraw;

    Data<openvdb::FloatGrid::Ptr> d_vdbgrid;
};

}
