SET(AGENT_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/agent/includes")
SET(AGENT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/agent/src")

SET(AGENT_COMPILE_FLAGS "-lcrypto -lssh -lcurl")
SET(RELEASE_FLAGS "-s")
SET(DEBUG_FLAGS "-ggdb -Wall")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${AGENT_COMPILE_FLAGS} ${DEBUG_FLAGS}")

include_directories( ${AGENT_INCLUDES_DIR} ${COMMON_INCLUDES_DIR})
link_libraries( crypto ssh curl )
add_executable(agent ${COMMON_SOURCE_DIR}/common.cpp ${COMMON_SOURCE_DIR}/tasking.cpp ${AGENT_SOURCE_DIR}/agent.cpp ${AGENT_SOURCE_DIR}/beacon.cpp ${AGENT_SOURCE_DIR}/main.cpp ${AGENT_SOURCE_DIR}/module.cpp ${AGENT_SOURCE_DIR}/shell.cpp)
