#pragma once

#include <cstdlib>
#include <sys/socket.h>
#include <array>
#include <string>
#include <optional>
#include <functional>
#include <string_view>
#include <linux/if_packet.h>
#include "Frame.hpp"

/*
 * Represents a physical ethernet port on a switch. This class encapsulates all of the low-level raw
 * socket logic that handles actually reading and writing frames. This prevents the need to use raw
 * pointers and unsafe C logic throughout the entire program.
 */
class EthernetPort {
public:
    /*
     * Name of the Linux interface this EthernetPort instance will bind a raw socket to. We need
     * this to be constructed prior to socket_fd and mac_address since their construction depends
     * on it.
     */
    const std::string interface_name;

protected:
    EthernetPort(const std::string& i, int s)
        : interface_name{i},
          socket_fd{s} {
    }

    static int initialize_raw_socket(std::string_view);

    /*
     * Size of the socket read buffer. Size based on the largest possible ethernet frame length,
     * including an optional VLAN tag.
     */
    static constexpr size_t READ_BUFFER_SIZE = 1522;

    // File descriptor for the raw socket used to capture and send frames
    const int socket_fd;

    // Raw buffer used for reading frames off the raw socket
    std::array<unsigned char, EthernetPort::READ_BUFFER_SIZE> read_buffer;

public:
    EthernetPort(const std::string&);
    virtual ~EthernetPort() {
    }

    virtual bool operator==(const EthernetPort&) const;

    virtual std::optional<Frame> receive_frame();
    virtual bool send_frame(const Frame&);
};
