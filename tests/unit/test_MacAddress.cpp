#include <array>
#include <gtest/gtest.h>
#include "MacAddress.hpp"

TEST(MacAddressTests, RepresentationTests) {
    MacAddress m1(0x11, 0x22, 0x33, 0x44, 0x55, 0x66);
    std::array<unsigned char, 6> expected_raw_octets{{0x11, 0x22, 0x33, 0x44, 0x55, 0x66}};
    EXPECT_EQ(m1.int_representation, 0x112233445566);
    EXPECT_EQ(m1.readable_string, "11:22:33:44:55:66");
    EXPECT_EQ(m1.raw_octets, expected_raw_octets);
}

TEST(MacAddressTests, IsBroadcastTest) {
    MacAddress m1(0x11, 0x22, 0x33, 0x44, 0x55, 0x66);
    EXPECT_FALSE(m1.is_broadcast());

    MacAddress m2(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
    EXPECT_TRUE(m2.is_broadcast());
}

TEST(MacAddressTests, EqualityTests) {
    MacAddress m1(0x11, 0x22, 0x33, 0x44, 0x55, 0x66);
    MacAddress m2(0x11, 0x22, 0x33, 0x44, 0x55, 0x66);
    MacAddress m3(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
    
    EXPECT_EQ(m1, m1);
    EXPECT_EQ(m1, m2);
    EXPECT_NE(m1, m3);
}

TEST(MacAddressTests, LessThanTests) {
    MacAddress m1(0x11, 0x22, 0x33, 0x44, 0x55, 0x66);
    MacAddress m2(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
    
    EXPECT_TRUE(m1 < m2);
    EXPECT_FALSE(m2 < m1);
    EXPECT_FALSE(m1 < m1);
}

