#pragma once

#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <utility>
#include <thread>
#include <cstdint>
#include <atomic>
#include <memory>
#include <gtest/gtest_prod.h>

#include "MacAddress.hpp"
#include "MacAddressHash.hpp"
#include "EthernetPort.hpp"
#include "Frame.hpp"

/*
 * Class encapsulating data structures and switching logic for a simulated layer 2 network switch.
 */
class Layer2Switch {
    FRIEND_TEST(Layer2SwitchTests, SwitchImplTests);
    FRIEND_TEST(Layer2SwitchTests, ReceiveFrameFailureTests);
    FRIEND_TEST(Layer2SwitchTests, SendFrameFailureTests);

private:
    /*
     * Maps a MAC address to a physical port. a.k.a, a CAM table. This table will be auto-populated
     * as frames pass through the switch.
     */
    std::unordered_map<MacAddress, std::shared_ptr<EthernetPort>, MacAddressHash> mac_address_table;

    // List of all simulated ethernet ports on this switch
    std::vector<std::shared_ptr<EthernetPort>> ports;

    // Queues up Frames acquired from various receiver threads
    std::queue<std::pair<Frame, std::shared_ptr<EthernetPort>>> input_queue;

    // Mutex for the input queue
    std::mutex input_queue_mutex;

    // Counts the number of frames received
    std::atomic_uint64_t received_frames_count;

    // Counts the number of frames sent
    std::atomic_uint64_t sent_frames_count;

    // Counts number of socket send errors while sending directly (i.e., not flooding)
    std::atomic_uint64_t send_errors_count;

    // Counts number of socket send errors while flooding
    std::atomic_uint64_t flood_errors_count;

    /*
     * Counts the number of times frames are "flooded", either from a MAC table lookup miss or
     * because a broadcast MAC was received.
     */
    std::atomic_uint64_t flood_count;

    // Counts the number of socket read errors
    std::atomic_uint64_t read_errors_count;

    void switch_impl();
    void metric_worker();
    void frame_receiver_worker_impl(const std::shared_ptr<EthernetPort>&);
    void frame_receiver_worker(const std::shared_ptr<EthernetPort>&);

public:
    Layer2Switch(const std::vector<std::shared_ptr<EthernetPort>>&);
    ~Layer2Switch();

    void start();
};
