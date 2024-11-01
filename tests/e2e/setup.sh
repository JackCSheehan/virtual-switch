#!/bin/bash
# Sets up a simulated network for end-to-end testing using Docker.

set -e

if [[ $1 == "" ]]; then
    echo "Usage: $0 <number of hosts to create>"
    exit 1
fi

source common.sh

HOST_COUNT=$1

if (( HOST_COUNT > 253 )); then
    echo "Host count must be <= 253 as .254 is reserved for the gateway and .255 is reserved for the broadcast."
    exit 1
fi

# Clean up after any past runs
./teardown.sh

# First, create a network to connect the simulated hosts
echo "Creating Docker network $DOCKER_NETWORK_NAME"
docker network create \
    -o com.docker.network.bridge.name=$DOCKER_BRIDGE_NAME \
    --subnet=172.0.0.0/24 \
    --ip-range=172.0.0.0/24 \
    --gateway=172.0.0.254 \
    $DOCKER_NETWORK_NAME

# Next, create the simulated hosts
echo "Creating $HOST_COUNT simulated host(s)"
for i in $(seq 1 $HOST_COUNT); do
    docker run \
        -itd \
        --network=$DOCKER_NETWORK_NAME \
        --name=$DOCKER_CONTAINER_PREFIX$i \
        alpine:latest
done

# Sever the bridge connecting the containers so we can test connecting the hosts with the virtual
# switch application
sudo ip link del dev $DOCKER_BRIDGE_NAME

