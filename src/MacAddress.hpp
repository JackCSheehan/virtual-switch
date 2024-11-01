#pragma once

#include <cstdint>
#include <array>
#include <string>

#include "MacAddressHash.hpp"

/*
 * Represents a MAC address.
 */
class MacAddress {
private:
    /*
     * Size of the string buffer used to hold the human-readable version of the MAC. It's written
     * this way for readability, and both clang and gcc will optimize this out to be a compile-time
     * constant at -O2 or above.
     */
    static constexpr size_t READABLE_STRING_BUFFER_SIZE = sizeof("00:00:00:00:00:00");

public:
    static std::string initialize_readable_string(const std::array<unsigned char, 6>&);
    static uint64_t initialize_int_representation(const std::array<unsigned char, 6>&);

    MacAddress(
        unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char
    );

    bool operator<(const MacAddress&) const;
    bool operator==(const MacAddress&) const;

    // Representation of the MAC via its 6 octets
    const std::array<uint8_t, 6> raw_octets;

    // A human-readable representation of the MAC, e.g. "00:00:00:00:00:00"
    std::string readable_string;

    // Integer representation of the MAC, e.g. 0x000000000000
    uint64_t int_representation;

    bool is_broadcast() const;

    friend struct MacAddressHash;
};
