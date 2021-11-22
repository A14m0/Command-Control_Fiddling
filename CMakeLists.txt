cmake_minimum_required(VERSION 3.10)

project(Global)

SET(COMMON_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/common/includes")
SET(COMMON_SOURCE_DIR "${PROJECT_SOURCE_DIR}/common/src")

add_subdirectory(server)
add_subdirectory(agent)
add_subdirectory(server/ssh_trans_new)
