// Copyright 2009-2024 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2024, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include "sst_config.h"

#include "sst/core/serialization/objectMap.h"

#include <cxxabi.h>

namespace SST {
namespace Core {
namespace Serialization {

// Static variable instantiation
std::vector<std::pair<std::string,ObjectMap*>> ObjectMap::emptyVars;


std::string
ObjectMap::demangle_name(const char* name)
{
    int status;

    char* demangledName = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    if (status == 0) {
        std::string ret(demangledName);
        std::free(demangledName);
        
        // // Find the position of the first '<' character
        // size_t pos = ret.find('<');
        // if (pos != std::string::npos) {
        //     // Extract the base class name
        //     return ret.substr(0, pos);
        // }
        
        // // If no '<' character found, return the original name
        return ret;
    } else {
        return "";
    }    
}


} // namespace Serialization
} // namespace Core
} // namespace SST
