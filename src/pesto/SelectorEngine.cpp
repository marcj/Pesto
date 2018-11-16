//
// Created by marc on 05.06.18.
//

#include "SelectorEngine.h"

Pesto::SelectorEngine::SelectorEngine(const Pesto::Element &root) : root(&root) {
}

Pesto::Element *Pesto::SelectorEngine::queryOne(std::string selectors) {
    auto result = query(selectors, 1);

    if (result.empty()) {
        return nullptr;
    }

    return result[0];
}

std::vector<Pesto::Element *> Pesto::SelectorEngine::query2(CSSRuleSelector &selector, unsigned long maxSize) {
    Pesto::Timer tmr;

    std::vector<Pesto::Element *> contextList;

    unsigned long parts = selector.parts.size();
    unsigned long contextRound = 0;

    if (parts == 0) {
        return contextList;
    }

    contextList.push_back((Pesto::Element *) root);
    for (auto &part: selector.parts) {
        contextRound++;
        bool isLastRound = contextRound == parts;
        bool isRootRound = contextRound == 1;

        std::vector<Pesto::Element *> new_context_list;

        for (auto &contextItem: contextList) {
            for (Pesto::Element *candidate = isRootRound ? contextItem : contextItem->firstChildElement();
                 candidate;
                 candidate =
                     part.op == '*' ? candidate->deepNextElement(contextItem) : candidate->nextElementSibling()) {

                if (isRootRound) {
                    //init contextRound
                    candidate->contextRound = contextRound;
                } else {
                    if (candidate->contextRound == contextRound) {
                        //we added this element already to this round
                        continue;
                    }
                }

                bool valid = true;

                if (!part.tagName.empty() && part.tagName != "*") {
                    if (candidate->tagName != part.tagName) {
                        continue;
                    }
                }

                if (!part.id.empty()) {
                    if (candidate->id != part.id) {
                        continue;
                    }
                }

                for (auto &clazz: part.classes) {
                    if (!candidate->hasClass(clazz)) {
                        valid = false;
                        break;
                    }
                }

                for (auto &clazz: part.pseudoClasses) {
                    if (!candidate->hasPseudoClass(clazz)) {
                        valid = false;
                        break;
                    }
                }

                if (!valid) {
                    continue;
                }

                candidate->contextRound = contextRound;
                new_context_list.push_back(candidate);

                if (isLastRound && maxSize && new_context_list.size() >= maxSize) {
                    return new_context_list;
                }
            }
        }

        contextList = new_context_list;
    }

//    printf("Query \"%s\" took %fms\n", selectors.c_str(), tmr.elapsed() * 1000);
    return contextList;
}

std::vector<Pesto::Element *> Pesto::SelectorEngine::query(const std::string &selectors, unsigned long maxSize) {
    auto selectorsSplit = Pesto::utils::split(selectors, ',');
    if (selectorsSplit.size() == 1) {
        auto selector = CSSParser::parseSelector(selectors);
        return query2(selector, maxSize);
    }

    std::vector<Pesto::Element *> list;
    for (auto &selectorString : selectorsSplit) {
        auto selector = CSSParser::parseSelector(selectorString);
        auto result = query2(selector, maxSize);

        //todo, make sure we don't add duplicate items
        list.insert(list.end(), result.begin(), result.end());
    }

    return list;
}

std::vector<Pesto::Element *> Pesto::SelectorEngine::findCandidates(CSSRuleSelector &selector) {
    Pesto::Timer tmr;

    std::vector<Pesto::Element *> contextList;

    unsigned long parts = selector.parts.size();
    unsigned long contextRound = 0;

    if (parts == 0) {
        return contextList;
    }

    contextList.push_back((Pesto::Element *) root);
    for (auto &part: selector.parts) {
        contextRound++;
        bool isRootRound = contextRound == 1;

        std::vector<Pesto::Element *> new_context_list;

        for (auto &contextItem: contextList) {
            for (Pesto::Element *candidate = isRootRound ? contextItem : contextItem->firstChildElement();
                 candidate;
                 candidate =
                     part.op == '*' ? candidate->deepNextElement(contextItem) : candidate->nextElementSibling()) {

                if (isRootRound) {
                    //init contextRound
                    candidate->contextRound = contextRound;
                } else {
                    if (candidate->contextRound == contextRound) {
                        //we added this element already to this round
                        continue;
                    }
                }

                if (!part.tagName.empty() && part.tagName != "*") {
                    if (candidate->tagName != part.tagName) {
                        continue;
                    }
                }

                if (!part.id.empty()) {
                    if (candidate->id != part.id) {
                        continue;
                    }
                }

                candidate->contextRound = contextRound;
                new_context_list.push_back(candidate);
            }
        }

        contextList = new_context_list;
    }

//    printf("findCandidates \"%s\" took %fms\n", selector.c_str(), tmr.elapsed() * 1000);
    return contextList;
}
