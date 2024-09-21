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

#ifndef SST_CORE_SERIALIZATION_IMPL_MAPPER_H
#define SST_CORE_SERIALIZATION_IMPL_MAPPER_H

#ifndef SST_INCLUDING_SERIALIZER_H
#warning \
    "The header file sst/core/serialization/impl/mapper.h should not be directly included as it is not part of the stable public API.  The file is included in sst/core/serialization/serializer.h"
#endif

#include "sst/core/serialization/objectMap.h"

#include <string>
#include <typeinfo>
#include <vector>

namespace SST {
namespace Core {
namespace Serialization {
namespace pvt {

class ser_mapper
{
    std::vector<ObjectMap*> obj_;
    bool next_item_read_only = false;
    
public:
    // template <class T>
    // void map_primitive(T& t, const char* name)
    void map_primitive(const std::string& name, ObjectMap* map)
    {
        obj_.back()->addVariable(name, map);
        if ( next_item_read_only ) {
            next_item_read_only = false;
            map->setReadOnly();
        }
    }

    void map_container(const std::string& name, ObjectMap* map)
    {
        obj_.back()->addVariable(name, map);
        for ( int i = 0; i < indent; ++i ) printf(" ");
        printf("Mapping container %s (type = %s), at address %p\n",
               map->getName().c_str(), ObjectMap::demangle_name(map->getType().c_str()).c_str(), map->getAddr());
        if ( next_item_read_only ) {
            next_item_read_only = false;
        }
    }

    void map_existing_object(const std::string& name, ObjectMap* map)
    {
        obj_.back()->addVariable(name, map);
        if ( next_item_read_only ) {
            next_item_read_only = false;
        }
    }
    
    // template <class T>
    // void map_hierarchy_start(T& t, const char* name)
    void map_hierarchy_start(const std::string& name, ObjectMap* map)
    {
        obj_.back()->addVariable(name, map);
        obj_.push_back(map);

        // printf("Mapping hierarchy %s (type = %s), at address %p\n", name, demangle_name(typeid(T).name()).c_str(), &t);
        indent++;
        if ( next_item_read_only ) {
            next_item_read_only = false;
        }
    }

    void map_hierarchy_end(/*const char* name*/)
    {
        // // Need to check to make sure we are ending the one we started
        // if ( obj_->getName() != name ) {
        //     // Error
        // }
        // obj_ = obj_->getParent();
        obj_.pop_back();
        indent--;
    }

    void init(ObjectMap* object)
    {
        obj_.push_back(object);
    }

    void reset()
    {
        obj_.clear();
    }

    /**
     * @brief pack_buffer
     * @param buf  Must be non-null
     * @param size Must be non-zero
     */
    void map_buffer(void* buf, int size);

    void setNextObjectReadOnly() { next_item_read_only = true; }
    
private:
    int indent = 0;
};

} // namespace pvt
} // namespace Serialization
} // namespace Core
} // namespace SST

#endif // SST_CORE_SERIALIZATION_IMPL_MAPPER_H
