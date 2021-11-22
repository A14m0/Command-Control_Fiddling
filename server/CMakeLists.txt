SET(SERVER_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/server/includes")
SET(SERVER_SOURCE_DIR "${PROJECT_SOURCE_DIR}/server/src")

SET(SERVER_COMPILE_FLAGS "-fpermissive -Wno-write-strings")
SET(RELEASE_FLAGS "-s")
SET(DEBUG_FLAGS "-ggdb -Wall")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SERVER_COMPILE_FLAGS} ${DEBUG_FLAGS}")

include_directories( ${COMMON_INCLUDES_DIR} ${SERVER_INCLUDES_DIR} )
link_libraries( pthread crypto dl )
add_executable(server ${COMMON_SOURCE_DIR}/common.cpp ${COMMON_SOURCE_DIR}/tasking.cpp ${SERVER_SOURCE_DIR}/main.cpp ${SERVER_SOURCE_DIR}/module.cpp ${SERVER_SOURCE_DIR}/netinst.cpp ${SERVER_SOURCE_DIR}/server.cpp)
