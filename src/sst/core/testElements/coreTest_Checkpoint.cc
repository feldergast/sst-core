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

#include "sst_config.h"

#include "sst/core/testElements/coreTest_Checkpoint.h"

#include <assert.h>

using namespace SST;
using namespace SST::CoreTestCheckpoint;

coreTestCheckpoint::coreTestCheckpoint(ComponentId_t id, Params& params) : Component(id)
{
    bool starter = params.find<bool>("starter", true);
    if ( starter ) { counter = params.find<uint32_t>("counter", 1000); }
    else {
        counter = 0;
    }
    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();

    link = configureLink("port", new Event::Handler2<coreTestCheckpoint, &coreTestCheckpoint::handleEvent>(this));
    sst_assert(link, CALL_INFO, -1, "Could not configure link");

    testString = params.find<std::string>("teststring", "");
}

coreTestCheckpoint::~coreTestCheckpoint() {}

void
coreTestCheckpoint::setup()
{
    if ( counter > 0 ) link->send(new coreTestCheckpointEvent(counter));
}

// incoming event is bounced back after decrementing its counter
// if counter is 0, end simulation
void
coreTestCheckpoint::handleEvent(Event* ev)
{
    coreTestCheckpointEvent* event = static_cast<coreTestCheckpointEvent*>(ev);

    if ( event->decCount() ) {
        getSimulationOutput().output("%s, OK to end simulation\n", getName().c_str());
        primaryComponentOKToEndSim();
    }
    getSimulationOutput().output(
        "%s, bounce %d, t=%" PRIu64 "\n", getName().c_str(), event->getCount(), getCurrentSimCycle());
    link->send(event);
}


void
coreTestCheckpoint::printStatus(Output& out)
{
    out.output("Component Status: %s, %p, %" PRIu32 ", %s\n", 
        getName().c_str(), link, counter, testString.c_str());
}


void 
coreTestCheckpoint::serialize_order(SST::Core::Serialization::serializer& ser)
{
    TraceFunction trace(CALL_INFO_LONG, false);
    SST::Component::serialize_order(ser);
    ser& link;
    trace.output("link = %p\n", link);
    ser& counter;
    trace.output("counter = %" PRIu32 "\n", counter);
}



// Element Libarary / Serialization stuff
