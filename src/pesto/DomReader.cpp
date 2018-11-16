//
// Created by marc on 04.06.18.
//

#include "DomReader.h"
#include "CSSParser.h"
#include "components/Registry.h"

void Pesto::DomReader::insertDomChildren(Pesto::Element &parent, tinyxml2::XMLNode &domParent) {
    for (tinyxml2::XMLNode *node = (tinyxml2::XMLNode *) domParent.FirstChild(); node; node = node->NextSibling()) {
        if (node->ToText()) {

            //add text
            Pesto::TextElement *element = new Pesto::TextElement();
            element->setParent(&parent);

            element->setText(node->ToText()->Value());
//            printf("%sText: %s\n", std::string(element->getDepth(), '-').c_str(), node->ToText()->Value());

        } else if (node->ToElement()) {

            if (strcmp("style", node->ToElement()->Name()) == 0) {
                Pesto::CSSParser *parser = new Pesto::CSSParser(node->ToElement()->GetText());
                parent.setCSSParser(parser);

                continue;
            }

            Pesto::Element *element = new Pesto::Element();
            element->setParent(&parent);
            element->tagName = node->ToElement()->Name();

            this->applyDomProperties(*element, *node->ToElement());

//            printf(
//                    ":%s%s\n",
//                    std::string(element->getDepth() * 2, ' ').c_str(),
//                    element->name().c_str()
//            );

            this->insertDomChildren(*element, *node);
        }
    }
}

void Pesto::DomReader::applyDomProperties(Pesto::Element &element, tinyxml2::XMLElement &domElement) {
    int v = 0;

    element.tagName = domElement.Name();

    if (domElement.Attribute("id")) {
        element.id = domElement.Attribute("id");
    }

    if (domElement.Attribute("cacheRender")) {
        element.cacheRender = true;
    }

    for (auto *attribute = domElement.FirstAttribute(); attribute; attribute = attribute->Next()) {
        element.attributes[attribute->Name()] = attribute->Value();

        if (strcmp("style", attribute->Name()) == 0) {
            element.setStyles(attribute->Value());
        }

        if (strcmp("class", attribute->Name()) == 0) {
            element.setClass(attribute->Value());
        }
    }
}

Pesto::DomReader::DomReader() {}

Pesto::Element *Pesto::DomReader::populate(const std::string &xml, Pesto::Element *parent) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.Parse(xml.c_str());

    if (error) {
        throw std::runtime_error(
            std::string("XML Error: ").append(tinyxml2::XMLDocument::ErrorIDToName(error)).c_str());
    }

    auto reader = new Pesto::DomReader();

    Pesto::Element *root = new Pesto::Element;
    root->setParent(parent);

    if (!doc.RootElement()) {
        throw std::runtime_error("Populate DOM from XML not possible since no root element was given");
    }

    if (doc.RootElement()->NextSiblingElement()) {
        //we have more than two root elements

        if (strcmp("style", doc.RootElement()->Name()) == 0 &&
            !doc.RootElement()->NextSiblingElement()->NextSiblingElement()) {
            //we have only two elements in it, style + element, so merge make root=element and return it

            Pesto::CSSParser *parser = new Pesto::CSSParser(doc.RootElement()->GetText());
            root->setCSSParser(parser);

            reader->applyDomProperties(*root, *doc.RootElement()->NextSiblingElement());
            reader->insertDomChildren(*root, *doc.RootElement()->NextSiblingElement());
            reader->applyControllers(*root);
            return root;
        }

        root->tagName = "body";
        tinyxml2::XMLNode *element = dynamic_cast<tinyxml2::XMLNode *>(&doc);
        reader->insertDomChildren(*root, *element);
        reader->applyControllers(*root);
        return root;
    }

    reader->applyDomProperties(*root, *doc.RootElement());
    reader->insertDomChildren(*root, *doc.RootElement());
    reader->applyControllers(*root);

    return root;
}

void Pesto::DomReader::applyControllers(Pesto::Element &element) {
    for (Element *current = &element; current; current = current->deepNextElement(&element)) {
        if (!current->controller) {
            if (utils::has(Registry::controllers, current->getTagName())) {

                auto controller = Registry::controllers[current->getTagName()];
                current->controller = controller(current);
                printf("Found controller for %s\n", current->getTagName().c_str());
                if (!current->controller->getStyle().empty()) {
                    Pesto::CSSParser *parser = new Pesto::CSSParser(current->controller->getStyle());
                    current->setCSSParser(parser);
                }
            }
        }
    }
}
