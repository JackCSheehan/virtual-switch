#!/bin/bash
# Script to teardown simulated Docker network
source common.sh

# First, remove the containers
docker ps -a --format '{{.Names}}' | grep $DOCKER_CONTAINER_PREFIX | xargs -r docker rm -f

docker_networks=$(docker network ls)

# Next, remove the network
if echo "$docker_networks" | grep -q "$DOCKER_NETWORK_NAME"; then
    docker network rm $DOCKER_NETWORK_NAME
fi


