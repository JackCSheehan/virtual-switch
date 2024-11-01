#include <iostream>
#include <syslog.h>
#include <chrono>
#include <cstdio>
#include <fstream>
#include "Layer2Switch.hpp"
#include "panic.hpp"

Layer2Switch::Layer2Switch(const std::vector<std::shared_ptr<EthernetPort>>& v)
    : ports{v},
      received_frames_count{0},
      sent_frames_count{0},
      flood_count{0},
      read_errors_count{0} {
    openlog("virtualswitch", LOG_CONS | LOG_NDELAY | LOG_PID, LOG_DAEMON);
}

Layer2Switch::~Layer2Switch() {
    closelog();
}

/*
 * This method encapsulates the actual implementation of the layer 2 switching logic. It waits for a
 * frame to be enqueued on the input queue, pulls that frame off, then uses the MAC address table to
 * decide how to switch the frame.
 */
void Layer2Switch::switch_impl() {
    // Wait for frames to be enqueued by the frame receiver workers
    while (input_queue.empty()) {}

    // Once we get a frame, pop it off the input queue and process it
    input_queue_mutex.lock();
    auto frame_port_pair = input_queue.front();
    input_queue.pop();
    input_queue_mutex.unlock();

    Frame frame = frame_port_pair.first;
    std::shared_ptr<EthernetPort> port = frame_port_pair.second;

    mac_address_table.insert_or_assign(frame.source_mac_address, port);

    /*
     * There are two cases where we'll want to "flood", i.e., send this frame out all the
     * ethernet ports except the port that we just received a frame from:
     * 1. If the destination MAC is a broadcast MAC, i.e. FF:FF:FF:FF:FF:FF. This will occur for
     *    certain protocols like ARP which are intended to be broadcast out.
     * 2. If we don't know which ethernet port corresponds to the destination MAC. In that case,
     *    we'll unicast flood with the intention of eventually getting a response from that MAC to
     *    populate the MAC address table with.
     */
    if (frame.destination_mac_address.is_broadcast() ||
        mac_address_table.find(frame.destination_mac_address) == mac_address_table.end()) {
        ++flood_count;

        for (const std::shared_ptr<EthernetPort>& current_port : ports) {
            // We already know the MAC of the port, so we don't need to flood to it
            if (*current_port == *port) {
                continue;
            }

            if (!current_port->send_frame(frame)) {
                ++flood_errors_count;
                syslog(
                    LOG_ERR, "Error while flooding frame to %s",
                    current_port->interface_name.c_str()
                );
                return;
            }
            ++sent_frames_count;
        }

        return;
    }

    // If we know there this frame should go, just send it
    std::shared_ptr<EthernetPort> destination_port =
        mac_address_table.at(frame.destination_mac_address);
    if (!destination_port->send_frame(frame)) {
        ++send_errors_count;
        syslog(
            LOG_ERR, "Error while sending frame from %s to %s", port->interface_name.c_str(),
            destination_port->interface_name.c_str()
        );
        return;
    }
    ++sent_frames_count;
}

/*
 * Simple async metric worker to occassionally log some metrics about the virtual switch to syslog.
 */
void Layer2Switch::metric_worker() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(60000));
        syslog(
            LOG_INFO,
            "Metrics report => "
            "received_frames_count: %ld, "
            "sent_frames_count: %ld, "
            "flood_count: %ld, "
            "read_errors_count: %ld, "
            "send_errors_count: %ld, "
            "flood_errors_count: %ld",
            received_frames_count.load(), sent_frames_count.load(), flood_count.load(),
            read_errors_count.load(), send_errors_count.load(), flood_errors_count.load()
        );
    }
}

/*
 * Implementation of a frame receiver worker.
 */
void Layer2Switch::frame_receiver_worker_impl(const std::shared_ptr<EthernetPort>& port) {
    std::optional<Frame> optional_frame = port->receive_frame();
    if (!optional_frame.has_value()) {
        ++read_errors_count;
        syslog(
            LOG_ERR, "Failed to receive from on port %s. Skipping", port->interface_name.c_str()
        );
        return;
    }

    const Frame frame = optional_frame.value();
    ++received_frames_count;

    // Add this frame to the input queue to be processed by the main switch loop
    std::lock_guard<std::mutex> g(input_queue_mutex);
    input_queue.emplace(frame, port);
}

/*
 * Simple std::thread worker to perform receiving of the frames from the given port. The actual
 * implementation of the logic has been split into a separate method to make unit testing easier.
 */
void Layer2Switch::frame_receiver_worker(const std::shared_ptr<EthernetPort>& port) {
    while (true) { frame_receiver_worker_impl(port); }
}

/*
 * Starts the actual frame switching logic. Note that this function blocks forever as it spawns the
 * main switching loop.
 */
void Layer2Switch::start() {
    syslog(LOG_INFO, "Starting virtual layer 2 switch process on %ld port(s)", ports.size());

    /*
     * We'll store all the worker threads in a vector in this scope because we need to ensure that
     * the std::jthread destructor isn't called at the end of each loop iteration. Since each
     * worker thread acts a daemon, the use of std::jthread here will cause the below for loop to
     * hang after the first iteration.
     *
     * Likewise, using std::thread will call std::terminate, since the destruction of std::thread
     * in the body of the for loop without having been joined will terminate the program.
     */
    std::vector<std::jthread> threads;

    // Spawn one thread per port to accept frames asynchronously
    for (std::shared_ptr<EthernetPort> port : ports) {
        syslog(LOG_INFO, "Starting frame receiver worker on %s", port->interface_name.c_str());
        threads.emplace_back(&Layer2Switch::frame_receiver_worker, this, port);
    }

    syslog(LOG_INFO, "Starting metrics worker");
    std::jthread metrics_worker(&Layer2Switch::metric_worker, this);

    // Main switch logic loop
    syslog(LOG_INFO, "Starting main switch loop");
    puts("Starting main switch loop");
    while (true) { switch_impl(); }
}
