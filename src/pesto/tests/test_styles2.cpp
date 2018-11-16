#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../Style2.h"
#include "../CSSParser.h"
#include "../DomReader.h"
#include "../CSSStyleApplier.h"
#include "../Document.h"

using namespace Pesto;

TEST_CASE("Styles2 basic") {
    Element2 element;

    REQUIRE(element.style.borderLeft.getWidth() == 0);

    element.style.borderLeft.setWidth(5);
    REQUIRE(element.style.borderLeft.getWidth() == 5);

    element.style.borderLeft.setWidth("15px");
    REQUIRE(element.style.borderLeft.getWidth() == 15);

    element.style.setProperty(element.styleState, "border-left-width", "25px");
    REQUIRE(element.style.borderLeft.getWidth() == 25);

    element.style.setProperty(element.styleState, "border-width", "35px");
    REQUIRE(element.style.borderLeft.getWidth() == 35);
}

TEST_CASE("Styles2 basic 2") {
    Element2 element;

    printf("Sizeof Element2=%zu\n", sizeof(Element2));
    printf("Sizeof CSSRule=%zu\n", sizeof(CSSRule));
    printf("Sizeof Style2=%zu\n", sizeof(Style2));
    printf("Sizeof Style2Property=%zu\n", sizeof(Style2Property));
    printf("Sizeof Style2FloatProperty=%zu\n", sizeof(Style2FloatProperty));
    printf("Sizeof std::string=%zu\n", sizeof(std::string));

    CSSRule block;
    block.properties["top"] = "1";
    REQUIRE(block.has("top"));
    element.styleState.registerRule(&block);

    REQUIRE(element.styleState.rules[0]->has("top"));
    REQUIRE(element.styleState.rules[0] == &block);
    REQUIRE(element.style.top.state == &element.styleState);

    REQUIRE(element.style.top.isDefined());
    REQUIRE(element.style.top.stringValue() == "1");
    REQUIRE(element.style.top.value() == 1);
}

TEST_CASE("Styles2 inlineBlock") {
    CSSRule block1{1};
    block1.properties["top"] = "3";

    Element2 element;
    element.styleState.registerRule(&block1);

    REQUIRE(element.style.top.isDefined());
    REQUIRE(element.style.top.value() == 3);

    element.style.top = "5";
    REQUIRE(block1.properties["top"] == "3");
    REQUIRE(element.style.top.isDefined());
    REQUIRE(element.style.top.value() == 5);

    element.style.top.unset();
    REQUIRE(block1.properties["top"] == "3");
    REQUIRE(element.style.top.isDefined());
    REQUIRE(element.style.top.value() == 3);

    element.style.left = "10";
    REQUIRE(element.style.left.isDefined());
    REQUIRE(element.style.left.value() == 10);

    element.style.left = "10%";
    REQUIRE(element.style.left.isDefined());
    REQUIRE(element.style.left.value(200) == 20);

    element.style.left.unset();
    REQUIRE(!element.style.left.isDefined());
    REQUIRE(element.style.left.value() == 0.0f);
}

TEST_CASE("Styles2 block weights, regular") {
    CSSRule block1{1};
    block1.properties["top"] = "1";

    CSSRule block2{2};
    block2.properties["top"] = "2";

    Element2 element;
    element.styleState.registerRule(&block1);
    element.styleState.registerRule(&block2);
    REQUIRE(element.style.top.isDefined());
    REQUIRE(element.style.top.value() == 2);

    //register order shouldn't depend on the outcome
    Element2 element2;
    element2.styleState.registerRule(&block2);
    element2.styleState.registerRule(&block1);
    REQUIRE(element2.style.top.isDefined());
    REQUIRE(element2.style.top.value() == 2);
}

TEST_CASE("Styles2 block weights, overwrite") {
    CSSRule block1{3};
    block1.properties["top"] = "1";

    CSSRule block2{2};
    block2.properties["top"] = "2";

    Element2 element;
    element.styleState.registerRule(&block1);
    element.styleState.registerRule(&block2);
    REQUIRE(element.style.top.isDefined());
    REQUIRE(element.style.top.value() == 1);

    //register order shouldn't depend on the outcome
    Element2 element2;
    element2.styleState.registerRule(&block2);
    element2.styleState.registerRule(&block1);
    REQUIRE(element2.style.top.isDefined());
    REQUIRE(element2.style.top.value() == 1);
}

TEST_CASE("Styles2 inheritance, don't interfere") {
    CSSRule block1{1};
    block1.properties["line-height"] = "5px";
    Element2 element;
    element.styleState.registerRule(&block1);

    CSSRule block2{1};
    block2.properties["line-height"] = "16px";
    Element2 parent;
    parent.styleState.registerRule(&block2);

    element.styleState.setParent(&parent.styleState);

    REQUIRE(element.style.lineHeight.isDefined());
    REQUIRE(element.style.lineHeight.value() == 5);

    REQUIRE(parent.style.lineHeight.isDefined());
    REQUIRE(parent.style.lineHeight.value() == 16);
}

TEST_CASE("Styles2 inheritance, do inherit") {
    Element2 parent;

    CSSRule block2{1};
    block2.properties["line-height"] = "16px";
    parent.styleState.registerRule(&block2);

    Element2 element;
    element.styleState.setParent(&parent.styleState);

    REQUIRE(element.style.lineHeight.isDefined());
    REQUIRE(element.style.lineHeight.value() == 16);

    REQUIRE(parent.style.lineHeight.isDefined());
    REQUIRE(parent.style.lineHeight.value() == 16);

    //break the inheritance, by overwriting child's value
    CSSRule block1{2};
    block1.properties["line-height"] = "5px";
    element.styleState.registerRule(&block1);

    REQUIRE(element.style.lineHeight.isDefined());
    REQUIRE(element.style.lineHeight.value() == 5);

    REQUIRE(parent.style.lineHeight.isDefined());
    REQUIRE(parent.style.lineHeight.value() == 16);

    //break with new block with higher weight
    CSSRule block3{3};
    block3.properties["line-height"] = "69px";
    element.styleState.registerRule(&block3);

    REQUIRE(element.style.lineHeight.isDefined());
    REQUIRE(element.style.lineHeight.value() == 69);

    REQUIRE(parent.style.lineHeight.isDefined());
    REQUIRE(parent.style.lineHeight.value() == 16);

    //make sure it doesnt break with lower block weight
    CSSRule block4{1};
    block4.properties["line-height"] = "110px";
    element.styleState.registerRule(&block4);

    REQUIRE(element.style.lineHeight.isDefined());
    REQUIRE(element.style.lineHeight.value() == 69); //still 69

    REQUIRE(parent.style.lineHeight.isDefined());
    REQUIRE(parent.style.lineHeight.value() == 16);
}


TEST_CASE("Styles2 Pesto::CSSParser::parseSelector") {
    CSSRuleSelector selector;

    selector = Pesto::CSSParser::parseSelector(".class-a");
    REQUIRE(selector.weight == 1'000);
    REQUIRE(selector.parts.size() == 1);
    REQUIRE(selector.parts[0].tagName == "");
    REQUIRE(selector.parts[0].id.empty());
    REQUIRE(selector.parts[0].classes[0] == "class-a");
    REQUIRE(selector.parts[0].pseudoClasses.empty());


    selector = Pesto::CSSParser::parseSelector("body div");
    REQUIRE(selector.weight == 2);
    REQUIRE(selector.parts[0].tagName == "body");
    REQUIRE(selector.parts[1].tagName == "div");

    REQUIRE(1'002 == CSSRuleSelector::WEIGHT_ELEMENT + CSSRuleSelector::WEIGHT_ELEMENT + CSSRuleSelector::WEIGHT_CLASS);

    selector = Pesto::CSSParser::parseSelector("body div.class");
    REQUIRE(selector.weight == 1'002);
    REQUIRE(selector.parts[0].tagName == "body");
    REQUIRE(selector.parts[1].tagName == "div");
    REQUIRE(selector.parts[1].classes[0] == "class");

    selector = Pesto::CSSParser::parseSelector("body.classA div.classB");
    REQUIRE(selector.weight == 2'002);
    REQUIRE(selector.parts[0].tagName == "body");
    REQUIRE(selector.parts[0].id == "");
    REQUIRE(selector.parts[0].classes[0] == "classA");
    REQUIRE(selector.parts[1].tagName == "div");
    REQUIRE(selector.parts[1].id == "");
    REQUIRE(selector.parts[1].classes[0] == "classB");

    selector = Pesto::CSSParser::parseSelector("body #bla");
    REQUIRE(selector.weight == 1'000'001);
    REQUIRE(selector.parts[0].tagName == "body");
    REQUIRE(selector.parts[1].tagName == "");
    REQUIRE(selector.parts[1].id == "bla");

    selector = Pesto::CSSParser::parseSelector("body #bla");
    REQUIRE(selector.weight == 1'000'001);
    REQUIRE(selector.parts[0].tagName == "body");
    REQUIRE(selector.parts[1].op == '*');
    REQUIRE(selector.parts[1].tagName == "");
    REQUIRE(selector.parts[1].id == "bla");

    selector = Pesto::CSSParser::parseSelector("body > .mowla h1");
    REQUIRE(selector.weight == 1'002);
    REQUIRE(selector.parts[0].tagName == "body");

    REQUIRE(selector.parts[1].op == '>');
    REQUIRE(selector.parts[1].tagName == "");
    REQUIRE(selector.parts[1].id == "");
    REQUIRE(selector.parts[1].classes[0] == "mowla");

    REQUIRE(selector.parts[2].op == '*');
    REQUIRE(selector.parts[2].tagName == "h1");
    REQUIRE(selector.parts[2].id == "");
    REQUIRE(selector.parts[2].classes.empty());

    selector = Pesto::CSSParser::parseSelector("body .mowla > * > div");
    REQUIRE(selector.weight == 1'003);
    REQUIRE(selector.parts[0].tagName == "body");
    REQUIRE(selector.parts[0].op == '*');

    REQUIRE(selector.parts[1].tagName == "");
    REQUIRE(selector.parts[1].classes[0] == "mowla");
    REQUIRE(selector.parts[1].op == '*');

    REQUIRE(selector.parts[2].tagName == "*");
    REQUIRE(selector.parts[2].op == '>');

    REQUIRE(selector.parts[3].tagName == "div");
    REQUIRE(selector.parts[3].op == '>');
}

TEST_CASE("Styles2 block constraints, whole picture") {
    Pesto::utils::setupErrorListener();

    std::string style = u8R"(
body div {
    padding: 5px;
}

body div h1 {
    line-height: 16px;
    color: white;
}

body div:hover h1 {
    line-height: 22px;
}

body div h1:hover {
    color: red;
}
)";

    std::string xml = u8R"(
<body>
    <div>
        <h1>Hello</h1>
        <div>
            <other>
                <h1>Hello</h1>
            </other>
        </div>
    </div>
    <div>
        <h1>Hello</h1>
    </div>
</body>
)";

    Pesto::Document document;
    Pesto::DomReader::populate(xml, &document);

    auto parser = new Pesto::CSSParser(style);
    document.setCSSParser(parser);

    Pesto::CSSStyleApplier applier(document);
    applier.applyStyles();

    auto h1s = document.query("h1");
    auto divs = document.query("div");

    REQUIRE(h1s.size() == 3);
    REQUIRE(divs.size() == 3);

    REQUIRE(parser->getRule(1)->has("line-height"));
    REQUIRE(h1s[0]->style.lineHeight.isDefined());
    REQUIRE(h1s[0]->style.lineHeight.value() == 16);

    divs[0]->styleState.addPseudoClass("hover");

    REQUIRE(h1s[0]->style.lineHeight.value() == 22);
    REQUIRE(h1s[1]->style.lineHeight.value() == 22);
    REQUIRE(h1s[2]->style.lineHeight.value() == 16);

    divs[1]->styleState.addPseudoClass("hover");

    REQUIRE(h1s[0]->style.lineHeight.value() == 22);
    REQUIRE(h1s[1]->style.lineHeight.value() == 22);
    REQUIRE(h1s[2]->style.lineHeight.value() == 16);

    divs[2]->styleState.addPseudoClass("hover");

    REQUIRE(h1s[0]->style.lineHeight.value() == 22);
    REQUIRE(h1s[1]->style.lineHeight.value() == 22);
    REQUIRE(h1s[2]->style.lineHeight.value() == 22);

    divs[0]->styleState.removePseudoClass("hover");

    REQUIRE(h1s[0]->style.lineHeight.value() == 16);
    REQUIRE(h1s[1]->style.lineHeight.value() == 22);
    REQUIRE(h1s[2]->style.lineHeight.value() == 22);

    divs[1]->styleState.removePseudoClass("hover");

    REQUIRE(h1s[0]->style.lineHeight.value() == 16);
    REQUIRE(h1s[1]->style.lineHeight.value() == 16);
    REQUIRE(h1s[2]->style.lineHeight.value() == 22);

    REQUIRE(h1s[0]->style.position.stringValue() == "static");

    REQUIRE(h1s[0]->style.color.value() == 0xffffffff);
    h1s[0]->styleState.addPseudoClass("hover");
    REQUIRE(h1s[0]->style.color.value() == 0xffff0000);
}


TEST_CASE("Styles2 block constraints, rules") {
    Pesto::utils::setupErrorListener();

    std::string style = u8R"(
.left {
    left: 5px;
}

another-element {
    right: 10px;
}
)";

    std::string xml = u8R"(
<body>
    <div class="left">
        <another-element>Hello</another-element>
    </div>
</body>
)";

    Pesto::Document document;
    Pesto::DomReader::populate(xml, &document);

    auto parser = new Pesto::CSSParser(style);
    document.setCSSParser(parser);

    Pesto::CSSStyleApplier applier(document);
    applier.applyStyles();

    auto left = document.queryOne(".left");
    auto anotherElement = document.queryOne("another-element");

    REQUIRE(anotherElement->styleState.rules.size() == 2);

    REQUIRE(anotherElement->styleState.rules[0]->has("left"));
    REQUIRE(!anotherElement->styleState.rules[1]->has("left"));

    REQUIRE(!anotherElement->styleState.rules[0]->active(&anotherElement->styleState));
    REQUIRE(anotherElement->styleState.rules[1]->active(&anotherElement->styleState));
}


TEST_CASE("Styles2 block font-family") {
    Pesto::utils::setupErrorListener();

    std::string style = u8R"(
.root {
}

button {
    right: 10px;
}
)";

    std::string xml = u8R"(
<div class="root">
    <button>Hello</button>
</div>
)";

    Pesto::Document document;
    Pesto::DomReader::populate(xml, &document);

    auto parser = new Pesto::CSSParser(style);
    document.setCSSParser(parser);

    Pesto::CSSStyleApplier applier(document);
    applier.applyStyles();

    auto root = document.queryOne(".root");
    auto button = document.queryOne("button");

    REQUIRE(button->styleState.rules.size() == 1);

    REQUIRE(!button->style.fontFamily.isDefined());
    REQUIRE(!root->style.fontFamily.isDefined());
}


TEST_CASE("Styles2 block same weight, definition order") {
    Pesto::utils::setupErrorListener();

    std::string style = u8R"(
.root {
    left: 5px;
}

.root {
    left: 10px;
}
)";

    std::string xml = u8R"(
<div class="root">
</div>
)";

    Pesto::Document document;
    Pesto::DomReader::populate(xml, &document);

    auto parser = new Pesto::CSSParser(style);
    document.setCSSParser(parser);

    Pesto::CSSStyleApplier applier(document);
    applier.applyStyles();

    auto root = document.queryOne(".root");

    REQUIRE(root->style.left.isDefined());
    REQUIRE(root->style.left.value() == 10);
}

