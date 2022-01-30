// Defines test cases for Common class functions
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "common.h"


//////////////////////////// TEST CASES ////////////////////////////
TEST_CASE("`Common` sanity checks") {
    SECTION("index_of"){
        REQUIRE(Common::index_of("baaaaa", 'b', 0) == 0);
        REQUIRE(Common::index_of("abaaaa", 'b', 0) == 1);
        REQUIRE(Common::index_of("aabaaa", 'b', 0) == 2);
        REQUIRE(Common::index_of("aaabaa", 'b', 0) == 3);
        REQUIRE(Common::index_of("aaaaba", 'b', 0) == 4);
        REQUIRE(Common::index_of("aaaaab", 'b', 0) == 5);
        REQUIRE(Common::index_of("baaaaa", 'b', 1) == 0);
        REQUIRE(Common::index_of("abaaaa", 'b', 1) == 1);
        REQUIRE(Common::index_of("aabaaa", 'b', 1) == 2);
        REQUIRE(Common::index_of("aaabaa", 'b', 1) == 3);
        REQUIRE(Common::index_of("aaaaba", 'b', 1) == 4);
        REQUIRE(Common::index_of("aaaaab", 'b', 1) == 5);
        REQUIRE(Common::index_of("ababaa", 'b', 0) == 1);
        REQUIRE(Common::index_of("ababaa", 'b', 1) == 3);
    }

    SECTION("substring"){
    }
    SECTION("clean_input"){
    }

    SECTION("str_split"){
    }

    SECTION("Network Files Structure") {
    }
    
}