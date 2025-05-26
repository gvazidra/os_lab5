#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "employ.h"
#include <set>

using namespace std;

TEST_CASE("Test isUniqueID function") {
    set<int> existingIDs = { 1, 2, 3 };
    REQUIRE(isUniqueID(existingIDs, 4) == true); 
    REQUIRE(isUniqueID(existingIDs, 2) == false); 
}

TEST_CASE("Test employee structure") {
    employee e = { 1, "Alice", 35.0 };
    REQUIRE(e.num == 1);
    REQUIRE(strcmp(e.name, "Alice") == 0);
    REQUIRE(e.hours == 35.0);
}
