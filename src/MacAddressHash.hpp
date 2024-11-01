#pragma once

#include "MacAddress.hpp"

class MacAddress;

/*
 * Custom hash implementation used for hashing MacAddress instances so they can be used as the key
 * in a std::unordered_map.
 */
struct MacAddressHash {
    size_t operator()(const MacAddress&) const;
};
