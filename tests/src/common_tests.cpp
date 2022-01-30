// Defines test cases for Common class functions
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "common.h"


//////////////////////////// TEST CASES ////////////////////////////
TEST_CASE("`Common` sanity checks") {
    SECTION("index_of"){
        REQUIRE(Common::index_of("aaaaaa", 'a', 0) == 0);
    }
    
}