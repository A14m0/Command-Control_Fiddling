SET(SERVER_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/server/includes")
SET(SSHT_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/server/ssh_trans_new")
SET(SERVER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/server/src")
SET(SSHT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/server/ssh_trans_new")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -shared -lcrypto -lssh -fpermissive -Wno-write-strings")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

SET(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
find_library(ssh REQUIRED)

include_directories(${COMMON_INCLUDES_DIR} ${SERVER_INCLUDES_DIR} ${SSHT_INCLUDES_DIR})

#add_library(crypto SHARED IMPORTED)
#add_library(ssh SHARED IMPORTED)


add_library(ssh_transport SHARED ${COMMON_SOURCE_DIR}/common.cpp 
    ${COMMON_SOURCE_DIR}/tasking.cpp ${SSHT_SOURCE_DIR}/ssh_transport.cpp 
    ${SERVER_SOURCE_DIR}/netinst.cpp ${SERVER_SOURCE_DIR}/server.cpp 
    ${SERVER_SOURCE_DIR}/module.cpp)

target_link_libraries(ssh_transport OpenSSL::Crypto ssh)

