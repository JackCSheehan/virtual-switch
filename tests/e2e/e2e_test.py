"""
End-to-end tests for the virtual switch.
"""

import os
import signal
import subprocess
import time
import docker
import pytest

def ping(container, ip: str) -> bool:
    """
    Helper function to ping an IP from inside a docker container.

    Args:
        container: Reference to the Docker container to ping from.
        ip: IP address to ping.
    Returns:
        True if the ping was successful and false otherwise.
    """
    status, _ = container.exec_run(f"ping -q -w 5 {ip}")

    # See: man ping
    # Ping returns exit status 0 on success
    return status == 0


def run_script(name: str, args: str = ""):
    """
    Helper function to run the given script name. We opt to call Bash scripts directly here rather
    than using the Docker SDK for setting up the network because we want to be able to share the
    setup logic between the E2E tests and a simple CMake rule without forcing a Python dependency
    on users who want to set up the sim network without running the E2E tests.
    """
    assert subprocess.run(f"./{name}.sh {args}", shell=True).returncode == 0


class VirtualSwitch:
    """
    Class to handle running and stopping a virtual switch process.

    Provides a context manager that will automatically handle killing the virtual switch after the
    manager is exited. For example:

        # Virtual switch is started
        with VirtualSwitch():
            # Do stuff here
            # ...
        # Virtual switch process is automatically killed
    """
    def __init__(self):
        self._switch_process: Optional[subprocess.Popen] = None

    def __enter__(self):
        self._switch_process = subprocess.Popen(
            ["./start_switch.sh"],
            shell=True,
            preexec_fn=os.setsid
        )
        time.sleep(5)
        self._switch_process.poll()

        # If returncode is not None, then the process has already exited
        assert self._switch_process.returncode is None

    def __exit__(self, exc_type, exc_val, exc_tb):
        os.killpg(os.getpgid(self._switch_process.pid), signal.SIGKILL)


class SimNetwork:
    """
    Simple class to handle setup and teardown of the simulated network used to perform end-to-end
    tests.

    This class provides a simple context manager that will automatically hadle tearing down the sim
    network. Simple use a with block and provide the number of hosts to create:

        # Sim network is created
        with SimNetwork(host_count=5):
            # Write assertions here
            # ...
        # Sim network is automatically torn down
    """
    def __init__(self, host_count: int):
        self._host_count = host_count

    def __enter__(self):
        run_script("setup", str(self._host_count))

    def __exit__(self, exc_type, exc_val, exc_tb):
        run_script("teardown")


def test_two_hosts():
    """
    A basic test case mimicking the worked example in the README. Sets up two hosts, runs the
    virtual switch, and ensures that they can ping each other.
    """
    with SimNetwork(host_count=2):
        docker_client = docker.from_env()
        pc1 = docker_client.containers.get("v-switch-pc1")
        pc2 = docker_client.containers.get("v-switch-pc2")

        # First, assert that we can't ping between the hosts
        assert not ping(pc1, "172.0.0.2")
        assert not ping(pc2, "172.0.0.1")

        with VirtualSwitch():
            # Now that virtual switch is running, assert that we _can_ ping between them
            assert ping(pc1, "172.0.0.2")
            assert ping(pc2, "172.0.0.1")


def test_four_hosts():
    """
    A larger tests that ensures we can switch traffic between multiple hosts.
    """
    with SimNetwork(host_count=4):
        docker_client = docker.from_env()
        pc1 = docker_client.containers.get("v-switch-pc1")
        pc2 = docker_client.containers.get("v-switch-pc2")
        pc3 = docker_client.containers.get("v-switch-pc3")
        pc4 = docker_client.containers.get("v-switch-pc2")

        with VirtualSwitch():
            assert ping(pc1, "172.0.0.2")
            assert ping(pc1, "172.0.0.3")
            assert ping(pc1, "172.0.0.4")

            assert ping(pc2, "172.0.0.1")
            assert ping(pc2, "172.0.0.3")
            assert ping(pc2, "172.0.0.4")

            assert ping(pc3, "172.0.0.1")
            assert ping(pc3, "172.0.0.2")
            assert ping(pc3, "172.0.0.4")

            assert ping(pc4, "172.0.0.1")
            assert ping(pc4, "172.0.0.2")
            assert ping(pc4, "172.0.0.3")

