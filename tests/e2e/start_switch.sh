#!/bin/bash
# Helper script to start the virtual switch for end-to-end tests.

source common.sh

docker_containers=$(docker ps --format '{{.Names}}' | grep $DOCKER_CONTAINER_PREFIX)

# String to hold a space-separated list of interface
interfaces=""

# First, we need to derive the veths made by the Docker network
for container in $docker_containers; do
    # First, figure out what this container's interface is indexed as
    interface_index=$(docker exec $container sh -c 'cat /sys/class/net/eth0/iflink')

    # Remove trailing carriage return to avoid problems when grepping with a colon
    interface_index=$(echo "$interface_index" | tr -d '\r')

    # Find the ip a entry for the interface
    interface_info=$(ip a | grep "^$interface_index: veth")

    # Finally, extract the interface name from the ip a entry
    interface_name=$(echo "$interface_info" | grep -o "veth.*@")

    # Since the last grep also includes a @, remove the last character
    interface_name=${interface_name::-1}

    interfaces+="$interface_name "
done

DEV_SWITCH_ARTIFACT_PATH="../../build/dev-switch"

# Enable raw sockets and call with the list of interfaces
sudo setcap cap_net_raw+ep $DEV_SWITCH_ARTIFACT_PATH
$DEV_SWITCH_ARTIFACT_PATH $interfaces

