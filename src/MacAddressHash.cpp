#include "MacAddressHash.hpp"

size_t MacAddressHash::operator()(const MacAddress& mac_address) const {
    return mac_address.int_representation;
}
