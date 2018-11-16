//
// Created by marc on 08.06.18.
//

#include "CSSStyleApplier.h"

Pesto::CSSStyleApplier::CSSStyleApplier(Pesto::Element &root) : root(root) {
}

void Pesto::CSSStyleApplier::applyStyles() {
    Pesto::Timer tmr;

    int i = 0;
    for (Pesto::Element *item = &root; item; item = item->deepNextElement(&root)) {
        i++;
        item->styleState.deAssignRules();
    }

    for (Pesto::Element *item = &root; item; item = item->deepNextElement(&root)) {
        auto parser = item->getCSSParser();

        if (parser) {
            Pesto::SelectorEngine selectorEngine(*item);

            for (auto &rule: *parser->getRules()) {
                auto foundElements = selectorEngine.findCandidates(rule.selector);

                for (auto &foundElement: foundElements) {
                    foundElement->styleState.registerRule(&rule);
                }
            }
        }
    }

    tmr.printElapsed("ApplyStyles for " + root.name(false) + " for " + std::to_string(i) + " items");
}