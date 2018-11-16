#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../core/utils.h"

TEST_CASE("Utils::split") {
    std::string string = "Hello World, Jo.";
    std::vector<std::string> tokens = Pesto::utils::split(string, ' ');

    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens.at(0) == "Hello");
    REQUIRE(tokens.at(1) == "World,");
    REQUIRE(tokens.at(2) == "Jo.");
}

TEST_CASE("Utils::replace") {
    std::string string = "Hello World, Jo.";

    auto result = Pesto::utils::replace(string, ' ', '-');

    REQUIRE(string == "Hello World, Jo.");
    REQUIRE(result == "Hello-World,-Jo.");
}

TEST_CASE("Utils::toHex") {
    REQUIRE(Pesto::utils::toHex(0xff) == "0x000000ff");
    REQUIRE(Pesto::utils::toHex(0xff00ff) == "0x00ff00ff");
    REQUIRE(Pesto::utils::toHex(0xff0000) == "0x00ff0000");
    REQUIRE(Pesto::utils::toHex(0x00ff0000) == "0x00ff0000");
    REQUIRE(Pesto::utils::toHex(0xf0ff000f) == "0xf0ff000f");
}