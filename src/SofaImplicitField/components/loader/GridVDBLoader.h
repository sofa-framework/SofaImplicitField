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

#include <sofa/core/loader/BaseLoader.h>
#include <SofaImplicitField/components/geometry/DiscreteGridField.h>
#include <openvdb/openvdb.h>

namespace sofaimplicitfield::component::io
{

namespace
{
using sofa::core::objectmodel::Data;
using sofa::type::Vec3;
using sofa::type::Vec3u;
using sofa::type::Vec3d;
using sofa::core::loader::BaseLoader;
}

class SOFA_SOFAIMPLICITFIELD_API GridVDBLoader : public BaseLoader
{
public:
    SOFA_CLASS(GridVDBLoader, BaseLoader);

    GridVDBLoader();
    ~GridVDBLoader() override;

    void init() override;
    bool load() override ;

    Data<openvdb::FloatGrid::Ptr> d_vdbgrid;
};

}

namespace sofa::core::objectmodel
{

/// Specialization for OpenVDB::FloaterGrid
template<> bool Data<openvdb::FloatGrid::Ptr>::read( const std::string&);
template<> void Data<openvdb::FloatGrid::Ptr>::printValue( std::ostream& ) const;
template<> std::string Data<openvdb::FloatGrid::Ptr>::getValueString() const;
template<> std::string Data<openvdb::FloatGrid::Ptr>::getDefaultValueString() const;

}