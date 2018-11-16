#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../xml/tinyxml2.h"
#include "../SelectorEngine.h"
#include "../DomReader.h"
#include "../Element.h"
#include "../TextElement.h"
#include "../core/utils.h"

Pesto::Element *e(Pesto::Node *n) {
    return dynamic_cast<Pesto::Element *>(n);
}

TEST_CASE("Selector element") {
    Pesto::utils::setupErrorListener();

    std::string xml = u8R"(
<root id="root" class="mowla">
  <div id="1">Hallo <strong id="2">World</strong> Yes</div>
  <div id="3">Mo</div>
  <div id="4">Mowla</div>
</root>
)";

    auto root = Pesto::DomReader::populate(xml);

    REQUIRE(root->tagName == "root");
    REQUIRE(root->getAttribute("class") == "mowla");
    REQUIRE(root->children.size() == 3);

    REQUIRE(e(root->firstChild)->id == "1");

    auto t = dynamic_cast<Pesto::TextElement*>(root->firstChild->firstChild);
    REQUIRE(t != nullptr);

    REQUIRE(t->text == "Hallo");
    REQUIRE(e(t->next)->id == "2");
    REQUIRE(e(t->parent)->id == "1");
    REQUIRE(e(root->firstChild->next)->id == "3");
    REQUIRE(e(root->lastChild->prev)->id == "3");
    REQUIRE(e(root->lastChild)->id == "4");

    delete root->firstChild->next; //delete id=3
    REQUIRE(e(root->firstChild->next)->id == "4");
    REQUIRE(e(root->lastChild->prev)->id == "1");
    REQUIRE(e(root->lastChild)->id == "4");
    REQUIRE(root->children.size() == 2);

}
