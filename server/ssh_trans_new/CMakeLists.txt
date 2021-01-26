SET(SERVER_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/server/includes")
SET(SSHT_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/server/ssh_trans_new")
SET(SERVER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/server/src")
SET(SSHT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/server/ssh_trans_new")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -shared -lcrypto -lssh -fpermissive")

include_directories(${SERVER_INCLUDES_DIR} ${SSHT_INCLUDES_DIR})

add_library(ssh_transport SHARED ${SSHT_SOURCE_DIR}/ssh_transport.cpp
    ${SERVER_SOURCE_DIR}/common.cpp ${SERVER_SOURCE_DIR}/netinst.cpp
    ${SERVER_SOURCE_DIR}/server.cpp ${SERVER_SOURCE_DIR}/module.cpp)
