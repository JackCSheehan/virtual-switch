#pragma once
#include <vector>
#include "MacAddress.hpp"

/*
 * A simple abstraction for a layer 2 frame.
 */
struct Frame {
    const MacAddress source_mac_address;
    const MacAddress destination_mac_address;

    /*
     * Raw buffer straight from the socket that this frame was parsed from. Note that we deep copy
     * the buffer into a member to allow Frame objects to own the data that represents them and not
     * be coupled to the lifetime or consistency of the read buffers written to by EthernetPort
     * objects.
     */
    const std::vector<unsigned char> buffer;

    Frame(const MacAddress& s, const MacAddress& d, const std::vector<unsigned char>& b)
        : source_mac_address{s},
          destination_mac_address{d},
          buffer{b} {
    }
};
