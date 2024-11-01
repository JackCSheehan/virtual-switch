file(GLOB E2E_SRCS *.py)

# Run end-to-end test cases
add_custom_target(e2e
    COMMAND
        python3 -m venv venv
    COMMAND
        venv/bin/pip install -r requirements.txt
    COMMAND
        venv/bin/python -m pytest .
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}
)

add_dependencies(e2e dev-switch)

# Set ups a demo simulated network environment for testing
add_custom_target(start-sim
    COMMAND ./setup.sh 2
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# Tears down the simulated network
add_custom_target(stop-sim
    COMMAND ./teardown.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

