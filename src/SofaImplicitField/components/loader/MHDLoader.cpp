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
#include <SofaImplicitField/components/loader/MHDLoader.h>
#include <SofaImplicitField/MHD.h>

#include <sofa/core/visual/VisualParams.h>

#include <sofa/core/ObjectFactory.h>
using sofa::core::RegisterObject ;

#include <fstream>

namespace sofaimplicitfield::component::io
{

GridMHDLoader::GridMHDLoader(){}
GridMHDLoader::~GridMHDLoader(){}

void GridMHDLoader::init()
{
    Inherit1::init();
    d_componentState = sofa::core::objectmodel::ComponentState::Valid;
}

bool GridMHDLoader::load()
{
    dmsg_info() << "Loading MHD file: " << d_filename;

    // -- Loading file
    const char* filename = d_filename.getFullPath().c_str();
    std::ifstream file(filename);
    if (!file.good())
    {
        msg_error() << "Cannot read file '" << d_filename << "'.";
        return false;
    }

    auto buffer = sofa::helper::getWriteOnlyAccessor(d_buffer);
    bool loadSucceeded = sofaimplicitfield::loader::loadGridFromMHD(filename,
                                                                    buffer->min, buffer->spacing,
                                                                    buffer->resolution, buffer->data);

    if(!loadSucceeded)
        return false;
    return true;
}

// Register in the Factory
void registerGridMHDLoader(sofa::core::ObjectFactory* factory)
{
    factory->registerObjects(sofa::core::ObjectRegistrationData("Load a MHD file storing a scalar field.")
    .add< GridMHDLoader >());
}

}
