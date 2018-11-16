#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../CSSParser.h"
#include "../core/utils.h"
#include "../Element.h"

TEST_CASE("CSS color") {
    Pesto::utils::setupErrorListener();

    SkColor color;

    color = Pesto::StrToColor("#000000");
    REQUIRE(color == 0xff000000);

    color = Pesto::StrToColor("#ff0000");
    REQUIRE(color == 0xffff0000);

    color = Pesto::StrToColor("#aaaaaa");
    REQUIRE(color == 0xffaaaaaa);

    color = Pesto::StrToColor("#ABCdef");
    REQUIRE(color == 0XFFABCdef);

    color = Pesto::StrToColor("#00ABCdef");
    REQUIRE(color == 0X00ABCdef);

}


TEST_CASE("CSS parser") {
    Pesto::utils::setupErrorListener();

    std::string style = u8R"(
mowla {
    color: red;
}

second {
    position: absolute;
    background: blue;
    font-size: 12px;
    left: 5px;
}
)";

    Pesto::CSSParser parser(style);
    REQUIRE(parser.getRules()->size() == 2);
    REQUIRE(parser.getRule(0)->properties.size() == 1);
    REQUIRE(parser.getRule(1)->properties.size() == 4);
}
