//
// Created by marc on 17.06.18.
//

#include <SkParse.h>
#import "Style2.h"
#include "CSSParser.h"

void Pesto::Style2State::registerRule(Pesto::CSSRule *rule) {
    rules.push_back(rule);

    std::sort(rules.begin(), rules.end(), [](Pesto::CSSRule *a, Pesto::CSSRule *b) -> bool {
        if (a->selector.weight == b->selector.weight) {
            return a->orderId > b->orderId;
        }

        return a->selector.weight > b->selector.weight;
    });

    invalidateCache();
}

std::string Pesto::Style2State::value(const std::string &name, bool inherit, std::string defaultValue) {
    if (Pesto::utils::has(cachedValue, name)) {
        return cachedValue[name];
    }


    //check blocks assigned to me
    if (hasInlineRule()) { //inline block is always active
        if (getInlineRule()->has(name)) {
            return cachedValue[name] = getInlineRule()->get(name);
        }
    }

    for (auto &rule: rules) {
        if (rule->active(this)) {
            if (rule->has(name)) {
                return cachedValue[name] = rule->get(name);
            }
        }
    }

    if (inherit && getParent()) {
        if (getParent()->hasInlineRule()) { //inline block is always active
            if (getParent()->getInlineRule()->has(name)) {
                return cachedValue[name] = getParent()->getInlineRule()->get(name);
            }
        }

        for (auto &rule: getParent()->rules) {
            if (rule->active(getParent())) {
                if (rule->has(name)) {
                    return cachedValue[name] = rule->get(name);
                }
            }
        }
    }

    return cachedValue[name] = defaultValue;
}

bool Pesto::Style2State::has(const std::string &name, bool inherit) {
    if (Pesto::utils::has(cachedDefined, name)) {
        return cachedDefined[name];
    }

    if (inlineRule) {
        //inline block is always active
        if (inlineRule->has(name)) {
            return (cachedDefined[name] = true);
        }
    }

    for (auto &rule: rules) {
        if (rule->active(this)) {
            if (rule->has(name)) {
                return (cachedDefined[name] = true);
            }
        }
    }

    if (inherit && parent) {
        if (parent->has(name, inherit)) {
            return (cachedDefined[name] = true);
        }
    }

    cachedDefined[name] = false;
    return false;
}

Pesto::CSSRule *Pesto::Style2State::getInlineRule() {
    if (inlineRule == nullptr) {
        inlineRule = new CSSRule(Pesto::CSSRuleSelector::WEIGHT_INLINE);
    }

    return inlineRule;
}

Pesto::Style2State::~Style2State() {
    if (tagNameOurs) {
        delete tagName;
    }
    if (idOurs) {
        delete id;
    }
    delete inlineRule;
    if (parent) {
        parent->removeChildren(this);
    }
}


uint32_t Pesto::StrToColor(const std::string &name) {
    uint32_t color = 0xff000000;
    SkParse::FindColor(name.c_str(), &color);
    return color;
}

std::string Pesto::ColorToString(uint32_t color) {
    return std::string(Pesto::utils::toStringHex(color));
}