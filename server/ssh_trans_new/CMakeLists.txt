SET(SERVER_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/server/includes")
SET(SSHT_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/server/ssh_trans_new")
SET(SERVER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/server/src")
SET(SSHT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/server/ssh_trans_new")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -shared -lcrypto -lssh -fpermissive -Wno-write-strings")

include_directories(${COMMON_INCLUDES_DIR} ${SERVER_INCLUDES_DIR} ${SSHT_INCLUDES_DIR})

add_library(ssh_transport SHARED ${COMMON_SOURCE_DIR}/common.cpp 
    ${SSHT_SOURCE_DIR}/ssh_transport.cpp ${SERVER_SOURCE_DIR}/netinst.cpp 
    ${SERVER_SOURCE_DIR}/server.cpp ${SERVER_SOURCE_DIR}/module.cpp)
