#include <vector>

#include "Layer2Switch.hpp"
#include "MacAddress.hpp"
#include "EthernetPort.hpp"
#include "panic.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PANIC("Too few arguments. Usage: %s <interface name>...\n", argv[0]);
    }

    // Consume the list of interfaces to bind the switch to
    std::vector<std::shared_ptr<EthernetPort>> ports;
    for (int i = 1; i < argc; ++i) { ports.push_back(std::make_shared<EthernetPort>(argv[i])); }

    Layer2Switch l2_switch(ports);
    l2_switch.start();

    return 0;
}
