SET(AGENT_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/agent/includes")
SET(AGENT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/agent/src")

SET(AGENT_COMPILE_FLAGS "-lcrypto -lssh -lcurl")
SET(RELEASE_FLAGS "-s")
SET(DEBUG_FLAGS "-ggdb -Wall")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${AGENT_COMPILE_FLAGS} ${DEBUG_FLAGS}")

include_directories( ${AGENT_INCLUDES_DIR} )
link_libraries( crypto ssh curl )
add_executable(agent ${AGENT_SOURCE_DIR}/agent.c ${AGENT_SOURCE_DIR}/b64.c ${AGENT_SOURCE_DIR}/beacon.c ${AGENT_SOURCE_DIR}/client.c ${AGENT_SOURCE_DIR}/shell.c)
