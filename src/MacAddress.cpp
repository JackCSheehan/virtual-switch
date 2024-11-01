#include <cstdio>
#include <iostream>

#include "MacAddress.hpp"

std::string MacAddress::initialize_readable_string(const std::array<unsigned char, 6>& raw_octets) {
    char raw_readable_string_buffer[MacAddress::READABLE_STRING_BUFFER_SIZE];
    snprintf(
        raw_readable_string_buffer, MacAddress::READABLE_STRING_BUFFER_SIZE,
        "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", raw_octets.at(0), raw_octets.at(1), raw_octets.at(2),
        raw_octets.at(3), raw_octets.at(4), raw_octets.at(5)
    );

    return {raw_readable_string_buffer};
}

uint64_t MacAddress::initialize_int_representation(const std::array<unsigned char, 6>& raw_octets) {
    return (uint64_t)raw_octets.at(0) << 40 | (uint64_t)raw_octets.at(1) << 32 |
           (uint64_t)raw_octets.at(2) << 24 | (uint64_t)raw_octets.at(3) << 16 |
           (uint64_t)raw_octets.at(4) << 8 | (uint64_t)raw_octets.at(5);
}

/*
 * Constructs a new MacAddress object from each of the 6 octets of the address. This constructor
 * is designed specifically work well with the ethhdr struct that can be parsed from a raw socket
 * buffer. For example:
 *  MacAddress m(
 *      eth_hdr->h_source[0],
 *      eth_hdr->h_source[1],
 *      eth_hdr->h_source[2],
 *      eth_hdr->h_source[3],
 *      eth_hdr->h_source[4],
 *      eth_hdr->h_source[5]
 *  );
 */
MacAddress::MacAddress(
    unsigned char octet1, unsigned char octet2, unsigned char octet3, unsigned char octet4,
    unsigned char octet5, unsigned char octet6
)
    : raw_octets{{octet1, octet2, octet3, octet4, octet5, octet6}},
      readable_string{MacAddress::initialize_readable_string(raw_octets)},
      int_representation{MacAddress::initialize_int_representation(raw_octets)} {
}

bool MacAddress::operator<(const MacAddress& other) const {
    return int_representation < other.int_representation;
}

bool MacAddress::operator==(const MacAddress& other) const {
    return int_representation == other.int_representation;
}

// Returns true if the stored MAC is a broadcast MAC (i.e., FF:FF:FF:FF:FF:FF) and false otherwise
bool MacAddress::is_broadcast() const {
    return int_representation == 0xFFFFFFFFFFFF;
}
