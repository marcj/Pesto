#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../xml/tinyxml2.h"
#include "../SelectorEngine.h"
#include "../DomReader.h"
#include "../core/utils.h"
#include "../CSSStyleApplier.h"

TEST_CASE("Selector simple") {
    Pesto::utils::setupErrorListener();

    std::string xml = u8R"(
<root>
  <div>Hallo <strong class="bla" id="2">World</strong> Yes</div>
  <div id="4" class="mowla">Mo</div>
  <div>Mowla</div>
</root>
)";

    auto root = Pesto::DomReader::populate(xml);
    Pesto::SelectorEngine engine(*root);

    std::vector<Pesto::Element *> list;

    list = engine.query("*");
    REQUIRE(list.size() == 5);

    list = engine.query("div");
    REQUIRE(list.size() == 3);

    list = engine.query("strong");
    REQUIRE(list.size() == 1);

    list = engine.query(".mowla");
    REQUIRE(list.size() == 1);

    list = engine.query("div.mowla");
    REQUIRE(list.size() == 1);

    list = engine.query("strong.mowla");
    REQUIRE(list.size() == 0);

    list = engine.query(".mowla, .bla");
    REQUIRE(list.size() == 2);
    REQUIRE(list[0]->id == "4");
    REQUIRE(list[1]->id == "2");
}

TEST_CASE("Selector more complex selector") {
    Pesto::utils::setupErrorListener();

    std::string xml = u8R"(
<myRoot>
  <div id="1">
    <span id="1-c"></span>
  </div>
  <div id="2">
    <span id="2-p">
      <span id="2-c"></span>
    </span>
    <div id="4">
      <span id="4-c"></span>
    </div>
    <div id="5" class="class-a">
      <span id="parentDeep">
        <span id="deep"></span>
      </span>
      <span id="second"></span>
    </div>
  </div>
</myRoot>
)";


    /**
     * myRoot
     *   div
     *     span
     *   div
     *     span
     *       span <
     *     div
     *       span
     *     div.class-a
     *       span#parentDeep
     *         span#deep <
     *       span#second
     *
     * div div span span: 1
     * div span {}
     * div > span {}
     * div > span > span {}
     * div + span > span {}
     * .class-a span {}
     * .class-a span:hover {}
     */

    auto root = Pesto::DomReader::populate(xml);

    REQUIRE(root->children.size() == 2);
    REQUIRE(root->children[0]->children.size() == 1);
    REQUIRE(root->children[0]->children[0]->children.size() == 0);

    REQUIRE(root->children[1]->children.size() == 3);
    REQUIRE(root->children[1]->children[0]->children.size() == 1);

    REQUIRE(root->children[1]->children[1]->children.size() == 1);
    REQUIRE(root->children[1]->children[2]->children.size() == 2);
    REQUIRE(root->children[1]->children[2]->children[0]->children.size() == 1);
    REQUIRE(root->children[1]->children[2]->children[1]->children.size() == 0);

    Pesto::SelectorEngine engine(*root);

    int elementCount = 0;
    for (Pesto::Element *item = root; item; item = item->deepNextElement()) {
        elementCount++;
    }
    REQUIRE(elementCount == 12);

    std::vector<Pesto::Element *> list;

    list = engine.query("*");
    REQUIRE(list.size() == 12);

    list = engine.query("myRoot *");
    REQUIRE(list.size() == 11);

    list = engine.query("div *");
    REQUIRE(list.size() == 9);

    list = engine.query("div");
    REQUIRE(list.size() == 4);

    list = engine.query("div", 1);
    REQUIRE(list.size() == 1);
    REQUIRE(list[0]->id == "1");

    list = engine.query("div div");
    REQUIRE(list.size() == 2);
    REQUIRE(list[1]->id == "5");

    list = engine.query("div div", 1);
    REQUIRE(list.size() == 1);
    REQUIRE(list[0]->id == "4");

    list = engine.query("div div span");
    REQUIRE(list.size() == 4);

    list = engine.query("div div span span");
    REQUIRE(list.size() == 1);

    list = engine.query("div div span > span");
    REQUIRE(list.size() == 1);

    list = engine.query("div div > span");
    REQUIRE(list.size() == 3);

    list = engine.query("div > span");
    REQUIRE(list.size() == 5);

    list = engine.query(".class-a");
    REQUIRE(list.size() == 1);

    list = engine.query(".class-a span");
    REQUIRE(list.size() == 3);

    list = engine.query(".class-a span span");
    REQUIRE(list.size() == 1);

    list = engine.query("#4-c");
    REQUIRE(list.size() == 1);

    list = engine.query(".class-a > span");
    REQUIRE(list.size() == 2);

    list = engine.query(".class-a > span", 1);
    REQUIRE(list.size() == 1);

    list = engine.query(".class-a > span span");
    REQUIRE(list.size() == 1);
}

TEST_CASE("Selector match") {
    Pesto::CSSParser parser("body div:hover h1 {}");
    auto rule1 = parser.getRule(0);

    Pesto::Style2State stateH1{"h1"};
    Pesto::Style2State stateDiv{"div"};
    Pesto::Style2State stateBody{"body"};

    stateH1.setParent(&stateDiv);
    stateDiv.setParent(&stateBody);

    //no match because div is not :hover'ed
    REQUIRE(!rule1->active(&stateH1));

    stateDiv.addPseudoClass("hover");
    REQUIRE(rule1->active(&stateH1));

    stateBody.addPseudoClass("hover");
    REQUIRE(rule1->active(&stateH1));

    stateDiv.removePseudoClass("hover");
    REQUIRE(!rule1->active(&stateH1));
}



TEST_CASE("Selector match 2") {
    Pesto::CSSParser parser(".left {top: 5px;}");
    auto rule1 = parser.getRule(0);

    Pesto::Style2State stateH1{"h1"};
    Pesto::Style2State stateDiv{"div"};

    stateDiv.addClass("left");
    stateH1.setParent(&stateDiv);

    //no match because div is not :hover'ed
    REQUIRE(!rule1->active(&stateH1));
    REQUIRE(rule1->active(&stateDiv));

    stateDiv.registerRule(rule1);

    REQUIRE(stateDiv.has("top"));
    REQUIRE(!stateH1.has("top"));

    Pesto::Style2 styleH1;
    Pesto::Style2 styleDiv;

    styleH1.linkWithState(&stateH1);
    styleDiv.linkWithState(&stateDiv);

    REQUIRE(styleDiv.top.isDefined());
    REQUIRE(styleDiv.top.value() == 5);

    REQUIRE(!styleH1.top.isDefined());
    REQUIRE(styleH1.top.value() == 0);
}

TEST_CASE("Selector match inheritance") {
    Pesto::CSSParser parser(".left {color: red}");
    auto rule1 = parser.getRule(0);

    Pesto::Style2State stateH1{"h1"};
    Pesto::Style2State stateDiv{"div"};

    stateDiv.addClass("left");
    stateH1.setParent(&stateDiv);

    //no match because div is not :hover'ed
    REQUIRE(!rule1->active(&stateH1));
    REQUIRE(rule1->active(&stateDiv));

    stateDiv.registerRule(rule1);

    REQUIRE(stateDiv.has("color"));
    REQUIRE(!stateH1.has("color"));

    Pesto::Style2 styleH1;
    Pesto::Style2 styleDiv;

    styleH1.linkWithState(&stateH1);
    styleDiv.linkWithState(&stateDiv);

    REQUIRE(!styleDiv.top.isDefined());
    REQUIRE(!styleH1.top.isDefined());

    REQUIRE(styleDiv.color.isDefined());
    REQUIRE(styleH1.color.isDefined());

    REQUIRE(styleDiv.color.value() == 0xffff0000);
    REQUIRE(styleH1.color.value() == 0xff000000);
}


//TEST_CASE("Selector rule resolver") {
//    Pesto::utils::setupErrorListener();
//
//    std::string xml = u8R"(
//<style>
//myRoot {
//    left: 5;
//    color: red;
//}
//
//#2 {
//    color: blue;
//}
//
//.class-a .deep {
//    color: green;
//}
//</style>
//<myRoot>
//  <div id="1">
//    <span></span>
//  </div>
//  <div id="2">
//    <span>
//      <span></span>
//    </span>
//    <div id="4">
//      <span></span>
//    </div>
//    <div id="5" class="class-a">
//      <span>
//        <span id="deep" class="deep"></span>
//      </span>
//      <span id="second"></span>
//    </div>
//  </div>
//</myRoot>
//)";
//
//
//    /**
//     * root
//     *   div
//     *     span
//     *   div
//     *     span
//     *       span <
//     *     div
//     *       span
//     *     div.class-a
//     *       span
//     *         span <
//     *       span
//     */
//
//    auto root = Pesto::DomReader::populate(xml);
//    Pesto::SelectorEngine engine(*root);
//
//    Pesto::CSSStyleApplier applier(*root);
//    applier.applyStyles();
//
////    REQUIRE(engine.items.size() == 12);
////    REQUIRE(engine.items[0]->getCSSParser() != nullptr);
//
//    REQUIRE(engine.queryOne("#deep") != nullptr);
//
//    REQUIRE(root->style.left.isDefined());
//    REQUIRE(root->style.left.value() == 5);
//
//    auto red = Pesto::toColor("red");
//    auto blue = Pesto::toColor("blue");
//    auto green = Pesto::toColor("green");
//
//    REQUIRE(root->style.color == red);
//    REQUIRE(root->style.color == 0xffff0000);
//
//    auto item1 = engine.queryOne("#1");
//    REQUIRE(item1 != nullptr);
//    REQUIRE(item1->id == "1");
//    REQUIRE(item1->style.color.isDefined());
//    REQUIRE(item1->style.color == red);
//    REQUIRE(item1->style.color == 0xffff0000);
//
//    auto item2 = engine.queryOne("#2");
//    REQUIRE(item2 != nullptr);
//    REQUIRE(item2->id == "2");
//
//    REQUIRE(item2->style.color.isDefined());
//    REQUIRE(item2->style.color == blue);
//    REQUIRE(item2->style.color == 0xff0000ff);
//
//    auto item5 = engine.queryOne("#5");
//    REQUIRE(item5 != nullptr);
//    REQUIRE(item5->id == "5");
//
//    REQUIRE(item5->style.color.isDefined());
//    REQUIRE(item5->style.color == blue);
//    REQUIRE(item5->style.color == 0xff0000ff);
//    REQUIRE(!item5->style.left.isDefined());
//
//    auto deepSpan = engine.queryOne("span.deep");
//    REQUIRE(deepSpan != nullptr);
//    REQUIRE(deepSpan->hasClass("deep"));
//
//    REQUIRE(deepSpan->style.color.isDefined());
//    REQUIRE(deepSpan->style.color == green);
//    REQUIRE(deepSpan->style.color == 0xff008000);
//    REQUIRE(!deepSpan->style.left.isDefined());
//}