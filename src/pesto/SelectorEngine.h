//
// Created by marc on 05.06.18.
//

#ifndef DEEPKIT_SELECTORENGINE_H
#define DEEPKIT_SELECTORENGINE_H

#include "Element.h"
#include "core/utils.h"
#include "core/Timer.cpp"

#include "xml/tinyxml2.h"

#include <string>
#include <vector>
#include <functional>

namespace Pesto {
    class SelectorEngine {

    public:
        std::vector<Pesto::Element *> items;
        const Pesto::Element *root = nullptr;

        explicit SelectorEngine(const Pesto::Element &root);

        Pesto::Element *queryOne(std::string selectors);

        /**
         * Finds elements where rule match (not considering rule constraints like classes, pseudo-classes)
         */
        std::vector<Pesto::Element *> findCandidates(CSSRuleSelector &selector);

        /**
         * Finds elements where rule match
         */
        std::vector<Pesto::Element *> query2(CSSRuleSelector &selector, unsigned long maxSize = 0);

        /**
         * Finds elements where rule match
         */
        std::vector<Pesto::Element *> query(const std::string &selectors, unsigned long maxSize = 0);
    };
}


#endif //DEEPKIT_SELECTORENGINE_H
