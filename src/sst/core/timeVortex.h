// Copyright 2009-2023 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2023, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef SST_CORE_TIMEVORTEX_H
#define SST_CORE_TIMEVORTEX_H

#include "sst/core/activityQueue.h"
#include "sst/core/module.h"
#include "sst/core/serialization/serialize_impl_fwd.h"
#include "sst/core/simulation_impl.h"

namespace SST {

class Output;

/**
 * Primary Event Queue
 */
class TimeVortex : public ActivityQueue
{
public:
    SST_ELI_DECLARE_BASE(TimeVortex)
    SST_ELI_DECLARE_INFO_EXTERN(ELI::ProvidesParams)
    SST_ELI_DECLARE_CTOR_EXTERN(SST::Params&)

    TimeVortex() { max_depth = MAX_SIMTIME_T; }
    ~TimeVortex() {}

    // Inherited from ActivityQueue
    virtual bool      empty() override                    = 0;
    virtual int       size() override                     = 0;
    virtual void      insert(Activity* activity) override = 0;
    virtual Activity* pop() override                      = 0;
    virtual Activity* front() override                    = 0;

    /** Print the state of the TimeVortex */
    virtual void     print(Output& out) const = 0;
    virtual uint64_t getMaxDepth() const { return max_depth; }
    virtual uint64_t getCurrentDepth() const = 0;
    virtual void     dbg_print(Output& out) { print(out); }

    virtual void serialize_order(SST::Core::Serialization::serializer& ser) { ser& max_depth; }

protected:
    uint64_t max_depth;
};

template <>
class SST::Core::Serialization::serialize_impl<TimeVortex*>
{

    template <class A>
    friend class serialize;
    void operator()(TimeVortex*& s, SST::Core::Serialization::serializer& ser)
    {
        switch ( ser.mode() ) {
        case serializer::SIZER:
        case serializer::PACK:
            ser& Simulation_impl::getSimulation()->timeVortexType;
            s->serialize_order(ser);
            break;
        case serializer::UNPACK:
            std::string tv_type;
            ser&        tv_type;
            Params      p;
            s = Factory::getFactory()->Create<TimeVortex>(tv_type, p);
            s->serialize_order(ser);
            break;
        }
    }
};

} // namespace SST

#endif // SST_CORE_TIMEVORTEX_H
