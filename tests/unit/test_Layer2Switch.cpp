#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <optional>
#include "Layer2Switch.hpp"

using ::testing::Return;
using ::testing::AtLeast;

class MockEthernetPort : public EthernetPort {
private:
    inline static int mock_socket_fd;

public:
    MockEthernetPort(const std::string& i) : EthernetPort{i, mock_socket_fd++} {}
    MOCK_METHOD(std::optional<Frame>, receive_frame, (), (override));
    MOCK_METHOD(bool, send_frame, (const Frame&), (override));
};

TEST(Layer2SwitchTests, SwitchImplTests) {
    auto mock_eth0 = std::make_shared<MockEthernetPort>("eth0");
    auto mock_eth1 = std::make_shared<MockEthernetPort>("eth1");
    std::vector<std::shared_ptr<EthernetPort>> mock_ports{
        mock_eth0,
        mock_eth1
    };
    Layer2Switch l2switch{mock_ports};

    EXPECT_CALL(*mock_eth0, receive_frame)
        .WillOnce(Return(
            Frame{
                MacAddress{
                    0x11, 0x11, 0x11, 0x11, 0x11, 0x11
                },
                MacAddress{
                    0x22, 0x22, 0x22, 0x22, 0x22, 0x22
                },
                {}
            }
        ))
        .WillOnce(Return(
            Frame{
                MacAddress{
                    0x22, 0x22, 0x22, 0x22, 0x22, 0x22
                },
                MacAddress{
                    0x11, 0x11, 0x11, 0x11, 0x11, 0x11
                },
                {}
            }
        ));

    EXPECT_CALL(*mock_eth0, send_frame)
        .WillOnce(Return(true));

    EXPECT_CALL(*mock_eth1, send_frame)
        .WillOnce(Return(true));

    // Fake out receiving a few frames and ensure they end up in the input queue
    l2switch.frame_receiver_worker_impl(l2switch.ports.at(0));
    l2switch.frame_receiver_worker_impl(l2switch.ports.at(0));

    ASSERT_EQ(l2switch.read_errors_count, 0);
    ASSERT_EQ(l2switch.received_frames_count, 2);
    ASSERT_EQ(l2switch.input_queue.size(), 2);

    // Check that first frame gets dequeued and flooded
    l2switch.switch_impl();
    ASSERT_EQ(l2switch.input_queue.size(), 1);
    ASSERT_EQ(l2switch.sent_frames_count, 1);
    ASSERT_EQ(l2switch.flood_count, 1);
    ASSERT_EQ(l2switch.send_errors_count, 0);
    ASSERT_EQ(l2switch.flood_errors_count, 0);

    // Check that second frame is not flooded, since we have the MAC entry from the first time
    l2switch.switch_impl();
    ASSERT_EQ(l2switch.input_queue.size(), 0);
    ASSERT_EQ(l2switch.sent_frames_count, 2);
    ASSERT_EQ(l2switch.flood_count, 1);
    ASSERT_EQ(l2switch.send_errors_count, 0);
    ASSERT_EQ(l2switch.flood_errors_count, 0);
}

TEST(Layer2SwitchTests, ReceiveFrameFailureTests) {
    auto mock_eth0 = std::make_shared<MockEthernetPort>("eth0");
    std::vector<std::shared_ptr<EthernetPort>> mock_ports{
        mock_eth0,
    };
    Layer2Switch l2switch{mock_ports};

    EXPECT_CALL(*mock_eth0, receive_frame)
        .WillOnce(Return(std::nullopt));

    // Ensure that counters work in the event of a receive failure
    l2switch.frame_receiver_worker_impl(l2switch.ports.at(0));
    ASSERT_EQ(l2switch.input_queue.size(), 0);
    ASSERT_EQ(l2switch.received_frames_count, 0);
    ASSERT_EQ(l2switch.read_errors_count, 1);
}

TEST(Layer2SwitchTests, SendFrameFailureTests) {
    auto mock_eth0 = std::make_shared<MockEthernetPort>("eth0");
    auto mock_eth1 = std::make_shared<MockEthernetPort>("eth1");
    std::vector<std::shared_ptr<EthernetPort>> mock_ports{
        mock_eth0,
        mock_eth1
    };
    Layer2Switch l2switch{mock_ports};

    EXPECT_CALL(*mock_eth0, receive_frame)
        .WillOnce(Return(
            Frame{
                MacAddress{
                    0x11, 0x11, 0x11, 0x11, 0x11, 0x11
                },
                MacAddress{
                    0x22, 0x22, 0x22, 0x22, 0x22, 0x22
                },
                {}
            }
        ))
        .WillOnce(Return(
            Frame{
                MacAddress{
                    0x22, 0x22, 0x22, 0x22, 0x22, 0x22
                },
                MacAddress{
                    0x11, 0x11, 0x11, 0x11, 0x11, 0x11
                },
                {}
            }
        ));

    EXPECT_CALL(*mock_eth0, send_frame)
        .WillOnce(Return(true));

    EXPECT_CALL(*mock_eth1, send_frame)
        .WillOnce(Return(true));

    l2switch.frame_receiver_worker_impl(l2switch.ports.at(0));
    l2switch.frame_receiver_worker_impl(l2switch.ports.at(0));

    // Ensure that failure on flooding increments metrics
    l2switch.switch_impl();
    ASSERT_EQ(l2switch.input_queue.size(), 1);
    ASSERT_EQ(l2switch.sent_frames_count, 1);
    ASSERT_EQ(l2switch.flood_count, 1);
    ASSERT_EQ(l2switch.send_errors_count, 0);
    ASSERT_EQ(l2switch.flood_errors_count, 0);

    // Ensure that error on regular send increments metrics
    l2switch.switch_impl();
    ASSERT_EQ(l2switch.input_queue.size(), 0);
    ASSERT_EQ(l2switch.sent_frames_count, 2);
    ASSERT_EQ(l2switch.flood_count, 1);
    ASSERT_EQ(l2switch.send_errors_count, 0);
    ASSERT_EQ(l2switch.flood_errors_count, 0);
}
