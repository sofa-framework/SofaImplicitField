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
#include <SofaImplicitField/components/geometry/SparseGridField.h>
#include <sofa/core/visual/VisualParams.h>

#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/tools/VolumeToMesh.h>

#include <sofa/core/ObjectFactory.h>
using sofa::core::RegisterObject ;


namespace sofa::component::geometry
{

SparseGridField::SparseGridField()
    : ScalarField(),
      d_debugDraw(initData(&d_debugDraw, false, "debugDraw", "show the values on the grid"))
    , d_vdbgrid(initData(&d_vdbgrid, "vdbgrid", "The VDB Sparse Grid to create a dense grid from."))

{
}

SparseGridField::~SparseGridField()
{
}

void SparseGridField::init()
{
    d_componentState = sofa::core::objectmodel::ComponentState::Valid;
}

void SparseGridField::draw(const sofa::core::visual::VisualParams* params)
{
    if(!d_debugDraw.getValue())
        return;
}

void SparseGridField::getValues(const std::vector<Vec3d>& positions, std::vector<double>& results)
{
    openvdb::FloatGrid::ConstAccessor accessor = d_vdbgrid.getValue()->getConstAccessor();
    openvdb::tools::GridSampler<openvdb::FloatGrid::ConstAccessor, openvdb::tools::BoxSampler>
        sampler(accessor, d_vdbgrid.getValue()->transform());

    results.reserve(positions.size());
    results.clear();
    for(auto position : positions)
    {
        results.push_back(sampler.wsSample(openvdb::Vec3R(position.x(), position.y(), position.z())));
    }
}

double SparseGridField::getValue(const Vec3d& position, int &)
{
    auto sampler = openvdb::tools::GridSampler<openvdb::FloatGrid,
                                               openvdb::tools::BoxSampler>{*(d_vdbgrid.getValue())};

    return sampler.wsSample(openvdb::Vec3R(position.x(), position.y(), position.z()));
}

Vec3d SparseGridField::getGradient(const Vec3d& position, int& domain)
{
    auto& grid = *(d_vdbgrid.getValue());
    auto sampler = openvdb::tools::GridSampler<openvdb::FloatGrid,
                                               openvdb::tools::BoxSampler>{grid};

    const double h = grid.voxelSize()[0];

    auto F = [&](const openvdb::Vec3d& p) -> double {
        return sampler.wsSample(p);
    };

    openvdb::Vec3d vdbpos{position.x(), position.y(), position.z()};

    const double fxp = F(vdbpos + openvdb::Vec3d(h, 0, 0));
    const double fxm = F(vdbpos - openvdb::Vec3d(h, 0, 0));

    const double fyp = F(vdbpos + openvdb::Vec3d(0, h, 0));
    const double fym = F(vdbpos - openvdb::Vec3d(0, h, 0));

    const double fzp = F(vdbpos + openvdb::Vec3d(0, 0, h));
    const double fzm = F(vdbpos - openvdb::Vec3d(0, 0, h));

    Vec3d g(
        (fxp - fxm) / (2.0 * h),
        (fyp - fym) / (2.0 * h),
        (fzp - fzm) / (2.0 * h)
        );

    return g;
}

void SparseGridField::getHessian(const Vec3d& position, type::Mat3x3d& result)
{
    auto& grid = *(d_vdbgrid.getValue());
    auto sampler = openvdb::tools::GridSampler<openvdb::FloatGrid,
                                               openvdb::tools::BoxSampler>{grid};

    const double h = grid.voxelSize()[0];
    openvdb::Vec3d vdbpos{position.x(), position.y(), position.z()};

    auto F = [&](const openvdb::Vec3d& p) -> double {
        return sampler.wsSample(p);
    };

    const double f = F(vdbpos);
    const double fxp = F(vdbpos + openvdb::Vec3d(h, 0, 0));
    const double fxm = F(vdbpos - openvdb::Vec3d(h, 0, 0));

    const double fyp = F(vdbpos + openvdb::Vec3d(0, h, 0));
    const double fym = F(vdbpos - openvdb::Vec3d(0, h, 0));

    const double fzp = F(vdbpos + openvdb::Vec3d(0, 0, h));
    const double fzm = F(vdbpos - openvdb::Vec3d(0, 0, h));

    const double fxx =
        (fxp - 2.0 * f + fxm) / (h * h);

    const double fyy =
        (fyp - 2.0 * f + fym) / (h * h);

    const double fzz =
        (fzp - 2.0 * f + fzm) / (h * h);

    const double fxy =
        (F(vdbpos + openvdb::Vec3d( h, h, 0))
         - F(vdbpos + openvdb::Vec3d( h,-h, 0))
         - F(vdbpos + openvdb::Vec3d(-h, h, 0))
         + F(vdbpos + openvdb::Vec3d(-h,-h,0)))
        / (4.0 * h * h);

    const double fxz =
        (F(vdbpos + openvdb::Vec3d( h, 0, h))
         - F(vdbpos + openvdb::Vec3d( h, 0,-h))
         - F(vdbpos + openvdb::Vec3d(-h, 0, h))
         + F(vdbpos + openvdb::Vec3d(-h, 0,-h)))
        / (4.0 * h * h);

    const double fyz =
        (F(vdbpos + openvdb::Vec3d(0, h, h))
         - F(vdbpos + openvdb::Vec3d(0, h,-h))
         - F(vdbpos + openvdb::Vec3d(0,-h, h))
         + F(vdbpos + openvdb::Vec3d(0,-h,-h)))
        / (4.0 * h * h);

    result = type::Mat3x3d({{
        fxx, fxy, fxz,
        fxy, fyy, fyz,
        fxz, fyz, fzz}});
}

// Register in the Factory
void registerSparseGridField(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("A discrete scalar field from a regular grid storing field value with interpolation.")
    .add< SparseGridField >());
}

}
