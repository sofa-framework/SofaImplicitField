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
******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <SofaImplicitField/config.h>
#include <SofaImplicitField/components/engine/FieldToGaussianSplat.h>

#include <Eigen/Eigenvalues>

#include <sofa/core/ObjectFactory.h>
#include <sofa/core/visual/VisualParams.h>
using sofa::core::RegisterObject;
using sofa::core::visual::VisualParams;

namespace sofaimplicitfield::component::engine
{

FieldToGaussianSplat::FieldToGaussianSplat()
    : d_resolution(initData(&d_resolution, Vec1u{10}, "resolution", "Number of samples in each dimension"))
    , d_min(initData(&d_min, {-1.0, -1.0, -1.0}, "min", "Minimum corner of the sampling grid"))
    , d_max(initData(&d_max, {1.0, 1.0, 1.0}, "max", "Maximum corner of the sampling grid"))
    , l_field(initLink("field", "The scalar field to sample"))
{
    addUpdateCallback("sample", {&d_resolution, &d_min, &d_max}, [this](const sofa::core::DataTracker&)
    {
        Vec1u resolution = d_resolution.getValue();
        Vec3d min = d_min.getValue();
        Vec3d max = d_max.getValue();

        sampleField();
        return core::objectmodel::ComponentState::Valid;
    }, {});
}

FieldToGaussianSplat::~FieldToGaussianSplat()
{
}

void FieldToGaussianSplat::init()
{
    if (!l_field.get())
    {
        msg_error() << "Missing scalar field to sample";
        d_componentState = core::objectmodel::ComponentState::Invalid;
        return;
    }

    sampleField();
    d_componentState = core::objectmodel::ComponentState::Valid;
}

void FieldToGaussianSplat::sampleField()
{
    auto field = l_field.get();
    if (!field)
    {
        msg_error() << "No scalar field linked";
        return;
    }

    auto positions = sofa::helper::getWriteAccessor(d_positions);
    auto normals = sofa::helper::getWriteAccessor(d_normals);
    auto resolution = d_resolution.getValue();
    auto min = d_min.getValue()+Vec3d{-0.001,-0.001,-0.001};
    auto max = d_max.getValue()+Vec3d{0.001,0.001,0.001};

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

    std::cout << getPathName() << " sampling the box surface " << std::endl;

    positions.reserve(resolution.x()*resolution.x()*6);
    auto space = (max-min)/resolution.x();

    for(unsigned int j=0;j<resolution.x();j++)
    {
        for(unsigned int k=0;k<resolution.x();k++)
        {
            positions.emplace_back(sofa::type::Vec3{min.x(), min.y()+space.y()*j, min.z()+space.z()*k});
            positions.emplace_back(sofa::type::Vec3{max.x(), min.y()+space.y()*j, min.z()+space.z()*k});

            positions.emplace_back(sofa::type::Vec3{min.x()+space.x()*j, min.y(), min.z()+space.z()*k});
            positions.emplace_back(sofa::type::Vec3{min.x()+space.x()*j, max.y(), min.z()+space.z()*k});

            positions.emplace_back(sofa::type::Vec3{min.x()+space.x()*j, min.y()+space.y()*k, min.z()});
            positions.emplace_back(sofa::type::Vec3{min.x()+space.x()*j, min.y()+space.y()*k, max.z()});
        }
    }
    normals.resize(positions.size());
    iterate();

    std::cout << getPathName() << " sampling the box surface::done " << positions.size() << std::endl;

    this->space = space.x()/2.0;
}

void FieldToGaussianSplat::iterate()
{
    std::cout << getPathName() << " sampling the box surface::iterate "  << std::endl;

    auto positions = sofa::helper::getWriteAccessor(d_positions);
    auto normals = sofa::helper::getWriteAccessor(d_normals);
    auto radius = sofa::helper::getWriteAccessor(d_radius);
    std::vector<double> p;
    std::vector<Vec3d> grads;

    assert(normals.size() == positions.size() && "Size does not match");
    p.resize(positions.size());

    std::vector<Vec3d> remaining = positions;
    positions.clear();
    normals.clear();
    radius.clear();
    for(int i=0;i<100 && remaining.size() > 0;i++)
    {
        for(auto it = remaining.begin(); it != remaining.end(); )
        {
            auto position = *it;
            auto value = l_field->getValue(position);
            auto grad = l_field->getGradient(position).normalized();
            if( value < 0.001 )
            {
                positions->push_back(position);
                normals->push_back(grad);
                radius->push_back(0.01);
                it = remaining.erase(it);
            }else{
                (*it) = (*it) - (grad * 0.01);
                it++;
            }
        }
    }
}

void FieldToGaussianSplat::computeBBox(const core::ExecParams* params, bool onlyVisible)
{
    if (onlyVisible) return;

    Vec3d min = d_min.getValue();
    Vec3d max = d_max.getValue();

    sofa::core::objectmodel::BaseComponent::computeBBox(params, onlyVisible);

    f_bbox.setValue({min, max});
}

void FieldToGaussianSplat::draw(const VisualParams* vparams)
{
    if (isComponentStateInvalid())
        return;

    auto positions = sofa::helper::getReadAccessor(d_positions);
    auto normals = sofa::helper::getReadAccessor(d_normals);
    auto radius = sofa::helper::getReadAccessor(d_radius);
    auto dt = vparams->drawTool();

    for(unsigned int i=0;i<positions.size();i++)
    {
        auto v = Vec3d{radius[i],0.0,0.0};
        dt->drawSplat(positions[i], normals[i], v, sofa::type::RGBAColor::red(),10);
    }

}

void registerFieldToGaussianSplat(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Samples a ScalarField to generates set of gaussian splat.")
    .add< FieldToGaussianSplat >());
}

}
