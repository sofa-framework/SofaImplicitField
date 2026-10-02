/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU General Public License as published by the Free  *
* Software Foundation; either version 2 of the License, or (at your option)   *
* any later version.                                                          *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for    *
* more details.                                                               *
*                                                                             *
* You should have received a copy of the GNU General Public License along     *
* with this program. If not, see <http://www.gnuSceneCreator_test.org/licenses/>.              *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/

#include <sofa/component/mapping/testing/MappingTestCreation.h>

#include <sofa/type/Vec.h>
using sofa::type::Vec3d ;

#include <SofaImplicitField/components/mapping/ScalarFieldMapping.h>
using sofaimplicitfield::mapping::ScalarFieldMapping ;

#include <SofaImplicitField/components/geometry/SphericalField.h>
using sofa::component::geometry::SphericalField ;

namespace
{

class ScalarFieldMappingTest : public sofa::mapping_test::Mapping_test<ScalarFieldMapping>
{
public:
    bool test()
    {
        auto field = sofa::core::objectmodel::New<SphericalField>();
        auto mapping = dynamic_cast<ScalarFieldMapping*>(this->mapping);
        mapping->l_field.set(field);

        field->d_centerSphere = Vec3d{0,0,0};
        field->d_radiusSphere = 1;

        Mapping::InVecCoord parentState;
        Mapping::OutVecCoord childState;

        // center pos
        parentState.push_back(sofa::type::Vec3{0.0,0.0,0.0});
        childState.push_back(sofa::type::Vec1{-1.0});

        // half radius along X, Y, Z
        parentState.push_back(sofa::type::Vec3{0.5,0.0,0.0});
        parentState.push_back(sofa::type::Vec3{0.0,0.5,0.0});
        parentState.push_back(sofa::type::Vec3{0.0,0.0,0.5});
        childState.push_back(sofa::type::Vec1{-0.75});
        childState.push_back(sofa::type::Vec1{-0.75});
        childState.push_back(sofa::type::Vec1{-0.75});

        // on the surface
        parentState.push_back(sofa::type::Vec3{1.0,0.0,0.0});
        parentState.push_back(sofa::type::Vec3{0.0,1.0,0.0});
        parentState.push_back(sofa::type::Vec3{0.0,0.0,1.0});
        childState.push_back(sofa::type::Vec1{0.0});
        childState.push_back(sofa::type::Vec1{0.0});
        childState.push_back(sofa::type::Vec1{0.0});

        // outside
        parentState.push_back(sofa::type::Vec3{2.0,0.0,0.0});
        parentState.push_back(sofa::type::Vec3{0.0,2.0,0.0});
        parentState.push_back(sofa::type::Vec3{0.0,0.0,2.0});
        childState.push_back(sofa::type::Vec1{3.0});
        childState.push_back(sofa::type::Vec1{3.0});
        childState.push_back(sofa::type::Vec1{3.0});

        assert(parentState.size() == childState.size() && "If size mismatch tests are broken");

        return runTest(parentState, childState);
    }
};

TEST_F( ScalarFieldMappingTest , test )
{
    setTestExecution(ScalarFieldMappingTest::TEST_ASSEMBLY_API, false);
    setTestExecution(ScalarFieldMappingTest::TEST_GEOMETRIC_STIFFNESS, false);
    errorMax = 1e2;
    ASSERT_TRUE(test());
}


}
