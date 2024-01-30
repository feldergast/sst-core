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

//#include <assert.h>

#include "sst_config.h"

#include "sst/core/testElements/coreTest_Serialization.h"

#include "sst/core/link.h"
#include "sst/core/objectSerialization.h"
#include "sst/core/rng/mersenne.h"
#include "sst/core/rng/rng.h"
#include "sst/core/warnmacros.h"

#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace SST {
namespace CoreTestSerialization {


template <typename T>
bool
checkSimpleSerializeDeserialize(T data)
{
    auto buffer = SST::Comms::serialize(data);
    T    result;
    SST::Comms::deserialize(buffer, result);
    return data == result;
};

template <typename T>
bool
checkContainerSerializeDeserialize(T& data)
{
    auto buffer = SST::Comms::serialize(data);
    T    result;
    SST::Comms::deserialize(buffer, result);

    if ( data.size() != result.size() ) return false;

    auto data_it   = data.begin();
    auto result_it = result.begin();

    // Only need to check one iterator since we already checked to
    // make sure they are equal size
    while ( data_it != data.end() ) {
        if ( *data_it != *result_it ) return false;
        ++data_it;
        ++result_it;
    }
    return true;
};

// For unordered containers
template <typename T>
bool
checkUContainerSerializeDeserialize(T& data)
{
    auto buffer = SST::Comms::serialize(data);
    T    result;
    SST::Comms::deserialize(buffer, result);

    if ( data.size() != result.size() ) return false;


    // Only need to check one iterator since we already checked to
    // make sure they are equal size
    for ( auto data_it = data.begin(); data_it != data.end(); ++data_it ) {
        bool match_found = false;
        for ( auto result_it = result.begin(); result_it != result.end(); ++result_it ) {
            if ( *data_it == *result_it ) {
                match_found = true;
                break;
            }
        }
        if ( !match_found ) return false;
    }
    return true;
};

// Classes to test pointer tracking
class pointed_to_class : public SST::Core::Serialization::serializable
{
    int value = -1;

public:
    pointed_to_class(int val) : value(val) {}
    pointed_to_class() {}

    int  getValue() { return value; }
    void setValue(int val) { value = val; }

    void serialize_order(SST::Core::Serialization::serializer& ser) override { ser& value; }

    ImplementSerializable(SST::CoreTestSerialization::pointed_to_class);
};

class shell : public SST::Core::Serialization::serializable
{
    int               value      = -10;
    pointed_to_class* pointed_to = nullptr;

public:
    shell(int val, pointed_to_class* ptc = nullptr) : value(val), pointed_to(ptc) {}
    shell() {}

    int  getValue() { return value; }
    void setValue(int val) { value = val; }

    pointed_to_class* getPointedTo() { return pointed_to; }
    void              setPointedTo(pointed_to_class* p) { pointed_to = p; }

    void serialize_order(SST::Core::Serialization::serializer& ser) override
    {
        ser& value;
        ser& pointed_to;
    }

    ImplementSerializable(SST::CoreTestSerialization::shell);
};

coreTestSerialization::coreTestSerialization(ComponentId_t id, UNUSED(Params& params)) : Component(id)
{
    Output& out = getSimulationOutput();

    rng = new SST::RNG::MersenneRNG();
    // Test serialization for various data types

    // Simple Data Types
    // int8, int16, int32, int64, uint8, uint16, uint32, uint64, float, double, pair<int, int>, string
    bool passed;

    passed = checkSimpleSerializeDeserialize<int8_t>(rng->generateNextInt32());
    if ( !passed ) out.output("ERROR: int8_t did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<int16_t>(rng->generateNextInt32());
    if ( !passed ) out.output("ERROR: int16_t did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<int32_t>(rng->generateNextInt32());
    if ( !passed ) out.output("ERROR: int32_t did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<int64_t>(rng->generateNextInt64());
    if ( !passed ) out.output("ERROR: int64_t did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<uint8_t>(rng->generateNextUInt32());
    if ( !passed ) out.output("ERROR: uint8_t did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<uint16_t>(rng->generateNextUInt32());
    if ( !passed ) out.output("ERROR: uint16_t did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<uint32_t>(rng->generateNextUInt32());
    if ( !passed ) out.output("ERROR: uint32_t did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<uint64_t>(rng->generateNextUInt64());
    if ( !passed ) out.output("ERROR: uint64_t did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<float>(rng->nextUniform() * 1000);
    if ( !passed ) out.output("ERROR: float did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<double>(rng->nextUniform() * 1000000);
    if ( !passed ) out.output("ERROR: double did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize<std::string>("test string");
    if ( !passed ) out.output("ERROR: string did not serialize/deserialize properly\n");

    passed = checkSimpleSerializeDeserialize(
        std::make_pair<int32_t, int32_t>(rng->generateNextInt32(), rng->generateNextInt32()));
    if ( !passed ) out.output("ERROR: pair<int32_t,int32_t> did not serialize/deserialize properly\n");

    // Ordered Containers
    // map, set, vector, list, deque
    std::map<int32_t, int32_t> map_in;
    for ( int i = 0; i < 10; ++i )
        map_in[rng->generateNextInt32()] = rng->generateNextInt32();
    passed = checkContainerSerializeDeserialize(map_in);
    if ( !passed ) out.output("ERROR: map<int32_t,int32_t> did not serialize/deserialize properly\n");

    std::set<int32_t> set_in;
    for ( int i = 0; i < 10; ++i )
        set_in.insert(rng->generateNextInt32());
    passed = checkContainerSerializeDeserialize(set_in);
    if ( !passed ) out.output("ERROR: set<int32_t> did not serialize/deserialize properly\n");

    std::vector<int32_t> vector_in;
    for ( int i = 0; i < 10; ++i )
        vector_in.push_back(rng->generateNextInt32());
    passed = checkContainerSerializeDeserialize(vector_in);
    if ( !passed ) out.output("ERROR: vector<int32_t> did not serialize/deserialize properly\n");

    std::list<int32_t> list_in;
    for ( int i = 0; i < 10; ++i )
        list_in.push_back(rng->generateNextInt32());
    passed = checkContainerSerializeDeserialize(list_in);
    if ( !passed ) out.output("ERROR: list<int32_t> did not serialize/deserialize properly\n");

    std::deque<int32_t> deque_in;
    for ( int i = 0; i < 10; ++i )
        deque_in.push_back(rng->generateNextInt32());
    passed = checkContainerSerializeDeserialize(deque_in);
    if ( !passed ) out.output("ERROR: deque<int32_t> did not serialize/deserialize properly\n");

    // Unordered Containers
    // unordered_map, unordered_set
    std::unordered_map<int32_t, int32_t> umap_in;
    for ( int i = 0; i < 10; ++i )
        umap_in[rng->generateNextInt32()] = rng->generateNextInt32();
    passed = checkUContainerSerializeDeserialize(umap_in);
    if ( !passed ) out.output("ERROR: unordered_map<int32_t,int32_t> did not serialize/deserialize properly\n");

    std::unordered_set<int32_t> uset_in;
    for ( int i = 0; i < 10; ++i )
        uset_in.insert(rng->generateNextInt32());
    passed = checkUContainerSerializeDeserialize(uset_in);
    if ( !passed ) out.output("ERROR: unordered_set<int32_t,int32_t> did not serialize/deserialize properly\n");


    // Containers to other containers

    {
        // There is one instance where we serialize a
        // std::map<std::string, uintptr_t> and deserialize as a
        // std::vector<std::pair<std::string, uintptr_t>>, so check that
        // here
        std::map<std::string, uintptr_t> map2vec_in = {
            { "s1", 1 }, { "s2", 2 }, { "s3", 3 }, { "s4", 4 }, { "s5", 5 }
        };
        std::vector<std::pair<std::string, uintptr_t>> map2vec_out;

        auto buffer = SST::Comms::serialize(map2vec_in);
        SST::Comms::deserialize(buffer, map2vec_out);

        passed = true;
        // Check to see if we get the same data
        for ( auto& x : map2vec_out ) {
            if ( map2vec_in[x.first] != x.second ) passed = false;
        }
        if ( passed && map2vec_in.size() != map2vec_out.size() ) passed = false;
        if ( !passed )
            out.output("ERROR: serializing as map<string,uintptr_t> and deserializing to "
                       "vector<pair<string,uintptr_t>> did not work properly\n");
    }

    // Need to test pointer tracking
    pointed_to_class* ptc10 = new pointed_to_class(10);
    pointed_to_class* ptc50 = new pointed_to_class(50);

    // First two will share a pointed to element
    shell* s1 = new shell(25, ptc10);
    shell* s2 = new shell(100, ptc10);

    // Next two are the same pointer
    shell* s3 = new shell(150, ptc50);
    shell* s4 = s3;

    std::vector<shell*> vec = { s1, s2, s3, s4 };

    SST::Core::Serialization::serializer ser;
    ser.enable_pointer_tracking();

    // Get the size
    ser.start_sizing();
    ser&   vec;
    size_t size = ser.size();

    char* buffer = new char[size];

    // Serialize
    ser.start_packing(buffer, size);
    ser& vec;

    // Deserialize
    std::vector<shell*> vec_out;
    ser.start_unpacking(buffer, size);
    ser& vec_out;

    // Now check the results

    // 0 and 1 should have the same object pointed to, but not be the
    // same object
    if ( vec_out[0] == vec_out[1] || vec_out[0]->getPointedTo() != vec_out[1]->getPointedTo() ) {
        out.output("ERROR: serializing objects with shared data using pointer tracking did not work properly\n");
    }

    if ( vec_out[2] != vec_out[3] ) {
        out.output("ERROR: serializing two pointers to the same object did not work properly\n");
    }
}


} // namespace CoreTestSerialization
} // namespace SST
