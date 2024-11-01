#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <string_view>

#include "EthernetPort.hpp"
#include "panic.hpp"

/*
 * Helper function to initialize a raw socket. Panics if the socket could not be initialized.
 * Returns the socket file descriptor.
 */
int EthernetPort::initialize_raw_socket(std::string_view interface_name) {
    int new_socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (new_socket_fd < 0) {
        PANIC(
            "Failed to open socket on interface %s: %s\n", interface_name.data(), strerror(errno)
        );
    }

    // Obtain the interface index
    ifreq index_ifreq;
    memset(&index_ifreq, 0, sizeof(ifreq));

    strncpy(index_ifreq.ifr_name, interface_name.data(), IFNAMSIZ - 1);
    if (ioctl(new_socket_fd, SIOCGIFINDEX, &index_ifreq) < 0) {
        PANIC(
            "Failed to obtain the interface index for %s. Reason: %s\n", interface_name.data(),
            strerror(errno)
        );
    }
    const int interface_index = index_ifreq.ifr_ifindex;

    // Configure the socket to only be bound to a specific interface
    sockaddr_ll socket_config;
    socket_config.sll_family = AF_PACKET;
    socket_config.sll_protocol = htons(ETH_P_ALL);
    socket_config.sll_ifindex = interface_index;

    if (bind(new_socket_fd, (sockaddr*)&socket_config, sizeof(sockaddr_ll)) < 0) {
        PANIC("Failed to bind\n");
    }

    return new_socket_fd;
}

/*
 * Constructs an ethernet port using the given interface name. This interface name will have a raw
 * socket bound to it to emulate the behavior of a physical port.
 */
EthernetPort::EthernetPort(const std::string& i)
    : interface_name{i},
      socket_fd{EthernetPort::initialize_raw_socket(interface_name)} {
    read_buffer.fill(0);
}

bool EthernetPort::operator==(const EthernetPort& other) const {
    return socket_fd == other.socket_fd;
}

/*
 * Receives the next frame from the bound interface and returns it as a Frame instance. Note that
 * this method blocks until the next packet arrives.
 */
std::optional<Frame> EthernetPort::receive_frame() {
    ssize_t read_length =
        recvfrom(socket_fd, read_buffer.data(), EthernetPort::READ_BUFFER_SIZE, 0, NULL, NULL);
    if (read_length < 0) {
        return {};
    }

    // If successful, parse the ethernet header and package it into a Frame instance
    ethhdr* eth_header = (ethhdr*)(read_buffer.data());

    return Frame{
        MacAddress(
            eth_header->h_source[0], eth_header->h_source[1], eth_header->h_source[2],
            eth_header->h_source[3], eth_header->h_source[4], eth_header->h_source[5]
        ),
        MacAddress(
            eth_header->h_dest[0], eth_header->h_dest[1], eth_header->h_dest[2],
            eth_header->h_dest[3], eth_header->h_dest[4], eth_header->h_dest[5]
        ),

        /*
         * Only copy as much data was read to we don't need to copy around READ_BUFFER_SIZE bytes
         * where not needed
         */
        std::vector(read_buffer.begin(), read_buffer.begin() + read_length)};
}

/*
 * Attempts to send the given Frame on this EthernetPort's interface. Returns true if sending was
 * successful and false otherwise.
 */
bool EthernetPort::send_frame(const Frame& frame) {
    ssize_t send_length = send(socket_fd, frame.buffer.data(), frame.buffer.size(), 0);
    return send_length >= 0;
}
