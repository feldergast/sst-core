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

#ifndef SST_CORE_CORETEST_CHECKPOINT_H
#define SST_CORE_CORETEST_CHECKPOINT_H

#include "sst/core/component.h"
#include "sst/core/event.h"
#include "sst/core/link.h"

namespace SST {
namespace CoreTestCheckpoint {

// Very simple starting case
// Expected to have two components in simulation.
// The components ping-pong an event until its count reaches 0

class coreTestCheckpointEvent : public SST::Event
{
public:
    coreTestCheckpointEvent() : SST::Event(), counter(1000) {}

    coreTestCheckpointEvent(uint32_t c) : SST::Event(), counter(c) {}

    ~coreTestCheckpointEvent() {}

    bool decCount()
    {
        if ( counter != 0 ) counter--;
        return counter == 0;
    }

    uint32_t getCount() { return counter; }

private:
    uint32_t counter;

    void serialize_order(SST::Core::Serialization::serializer& ser) override
    {
        Event::serialize_order(ser);
        ser& counter;
    }

    ImplementSerializable(SST::CoreTestCheckpoint::coreTestCheckpointEvent);
};


class coreTestCheckpoint : public SST::Component
{
public:
    SST_ELI_REGISTER_COMPONENT(
        coreTestCheckpoint,
        "coreTestElement",
        "coreTestCheckpoint",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "CoreTest Test Checkpoint",
        COMPONENT_CATEGORY_UNCATEGORIZED
    )

    SST_ELI_DOCUMENT_PARAMS(
        { "starter", "Whether this component initiates the ping-pong", "T"},
        { "count", "Number of times to bounce the message back and forth", "1000" },
        { "teststring", "A test string", ""}
    )

    SST_ELI_DOCUMENT_PORTS(
        {"port", "Link to the other coreTestCheckpoint", { "coreTestElement.coreTestCheckpointEvent", "" } }
    )

    coreTestCheckpoint(ComponentId_t id, SST::Params& params);
    ~coreTestCheckpoint();

    void setup();
    
    void printStatus(Output& out) override;

    // Serialization functions and macro
    coreTestCheckpoint() : Component() {} // For serialization only
    void serialize_order(SST::Core::Serialization::serializer& ser) override;
    ImplementSerializable(SST::CoreTestCheckpoint::coreTestCheckpoint)

private:
    void handleEvent(SST::Event* ev);

    SST::Link* link;
    uint32_t   counter; // Unused after setup
    std::string testString; // Test that string got serialized

};

} // namespace CoreTestCheckpoint
} // namespace SST

#endif // SST_CORE_CORETEST_CHECKPOINT_H
