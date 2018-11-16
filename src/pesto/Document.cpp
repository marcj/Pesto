//
// Created by marc on 01.06.18.
//

#include "Document.h"
#include "xml/tinyxml2.h"
#include "DomReader.h"
#include "CSSStyleApplier.h"

#include <tuple>

void Pesto::Document::tick(SkCanvas *canvas, double diff) {
    if (canvas == nullptr) {
        throw std::runtime_error("Document has no canvas.");
    }

    if (restyleNeeded) {
        CSSStyleApplier applier(*this);
        applier.applyStyles();
        restyleNeeded = false;
    }

//    std::vector<Node *> needsCalculation;
    std::vector<Element *> needsRedraw;
    std::vector<Element *> cacheRender;

    for (auto node = this->deepNext(this); node; node = node->deepNext(this)) {
//        if (node->recalculationNeeded) {
//            needsCalculation.push_back(node);
//        }
        if (node->redrawNeeded && node->isElementNode()) {
            needsRedraw.push_back(dynamic_cast<Element *>(node));
        }

        if (node->isElementNode() && node->isCacheRenderingEnabled()) {
            cacheRender.push_back(dynamic_cast<Element *>(node));
        }
    }

    if (recalculationNeeded) {
        innerBounds.x = 0;
        innerBounds.y = 0;
        innerBounds.maxWidth = size.width;
        innerBounds.maxHeight = size.height;
        innerBounds.height = 0;
        innerBounds.width = 0;

        //when we do this, it is required that all children do calculateLayout as well no matter if necessary or not.
        lines.clear();

        //first position != absolute && text
        for (auto &&item : children) {
            if (item->isElementNode()) {
                auto element = dynamic_cast<Pesto::Element *>(item);
                if (element->style.position != "auto") {
                    //absolute comes after static/relative, so we know dimensions (left/right) of absolute elements
                    item->calculateLayout(canvas);
                }
            } else {
                //calc text right away.
                item->calculateLayout(canvas);
            }
        }

        //now position=absolute
        for (auto &&item : children) {
            if (item->isElementNode()) {
                auto element = dynamic_cast<Pesto::Element *>(item);
                if (element->style.position == "absolute") {
                    //now absolute elements
                    item->calculateLayout(canvas);
                }
            }
        }

        recalculationNeeded = false;
    }

    drawFromCache(canvas, diff);

    for (auto &&item: cacheRender) {
        item->drawFromCache(canvas, diff);
    }
}

std::vector<Pesto::Element *> Pesto::Document::query(const std::string &selector) {
    auto engine = new Pesto::SelectorEngine(*this);

    return engine->query(selector);
}

Pesto::Element *Pesto::Document::queryOne(const std::string &selector) {
    auto engine = new Pesto::SelectorEngine(*this);

    return engine->queryOne(selector);
}

Pesto::Element *Pesto::Document::fromXML(const std::string &xml) {
    auto root = Pesto::DomReader::populate(xml, this);

    CSSStyleApplier applier(*root);
    applier.applyStyles();

    return root;
}

Pesto::Document::Document(Pesto::Window *window) : window(window) {
    size = Size::create(300, 300);
    tagName = "document";

    //we are the base offset
    style.position = "absolute";
    style.overflow = "auto";
}

Pesto::Document::~Document() {
    auto copy = children;
    for (auto &&item : copy) {
        delete item;
    }

    children.clear();
}

Pesto::Element *Pesto::Document::createElement(std::string tagName) {
    Pesto::Element *element = new Pesto::Element();
    element->setParent(this);
    element->tagName = tagName;

    return element;
}

void Pesto::Document::updateDepth() {
    depth = 0;
}

void Pesto::Document::restyle() {
    restyleNeeded = true;
}

Pesto::Document *Pesto::Document::getDocument() {
    return this;
}

bool Pesto::Document::isCacheRenderingEnabled() {
    return true;
}

Pesto::Window *Pesto::Document::getWindow() {
    return window;
}