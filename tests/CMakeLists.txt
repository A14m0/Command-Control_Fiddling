SET(TESTS_INCLUDES_DIR "${PROJECT_SOURCE_DIR}/tests/includes")
SET(TESTS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/tests/src")

include_directories( ${COMMON_INCLUDES_DIR} ${TESTS_INCLUDES_DIR} )

link_libraries( crypto )

add_executable(tests ${TESTS_SOURCE_DIR}/common_tests.cpp ${COMMON_SOURCE_DIR}/common.cpp)

add_test(NAME tests COMMAND tests)
