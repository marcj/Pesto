//
// Created by marc on 17.06.18.
//

#ifndef DEEPKIT_STYLE2_H
#define DEEPKIT_STYLE2_H

#import <string>
#import <vector>
#import <unordered_map>
#include <deque>

#include "core/utils.h"
#include "CSSParser.h"

namespace Pesto {
    enum class CSSStyleTextAlign {
        left,
        center,
        right
    };
    enum class CSSStyleVerticalAlign {
        top,
        middle,
        bottom
    };

    enum class CSSStyleWhiteSpace {
        normal,  //remove whitespace,   auto-wrap when necessary
        nowrap,  //remove whitespace,   no auto-wrap (only at <br>)
        pre,     //preserve whitespace, no auto-wrap (only at \n)
        preLine, //remove whitespace,   auto-wrap + at \n
        preWrap, //preserve whitespace, auto-wrap + at \n
    };

    enum class CSSStyleOverflow {
        visible,
        hidden,
        scroll,
        auto_
    };

    enum class CSSStyleDisplay {
        block,
        inlines
    };

    enum class CSSStyleBorderType {
        solid,
        dotted
    };

    enum class CSSStylePosition {
        statics,
        absolute,
        relative,
    };

    class Style2State {
        std::vector<std::string> pseudoClasses;
        std::vector<std::string> classes;

        std::unordered_map<std::string, std::string> cachedValue;
        std::unordered_map<std::string, bool> cachedDefined;

        CSSRule *inlineRule = nullptr;
        Style2State *parent = nullptr;

        std::vector<Style2State *> children;
    public:
        std::string *tagName = nullptr;
        bool tagNameOurs = false;
        std::string *id = nullptr;
        bool idOurs = false;

        unsigned long cacheRound = 1;

        //will be sorted the right way, so most important weight is at the top
        std::vector<CSSRule *> rules;

        Style2State() = default;

        ~Style2State();

        explicit Style2State(const std::string &tagName) {
            if (!tagName.empty()) {
                this->tagName = new std::string(tagName);
                tagNameOurs = true;
            }
        }

        Style2State(const std::string &tagName, const std::string &id) {
            if (!tagName.empty()) {
                this->tagName = new std::string(tagName);
                tagNameOurs = true;
            }

            if (!id.empty()) {
                this->id = new std::string(id);
                idOurs = true;
            }
        }

        void removeChildren(Style2State *child) {
            utils::remove(children, child);
        }

        void addChildren(Style2State *child) {
            children.push_back(child);
        }

        void setParent(Style2State *parent) {
            if (this->parent) {
                this->parent->removeChildren(this);
            }

            if (!parent) {
                this->parent = nullptr;
                return;
            }

            if (this->parent != parent) {
                this->parent = parent;
                parent->addChildren(this);
            }
        }

        Style2State *getParent() {
            return parent;
        }

        void invalidateCache() {
            cachedDefined.clear();
            cachedValue.clear();
            cacheRound++;
            for (auto &child: children) {
                child->invalidateCache();
            }
        }

        bool hasInlineRule() {
            return inlineRule != nullptr;
        }

        CSSRule *getInlineRule();

        std::string value(const std::string &name, bool inherit = false, std::string defaultValue = "");

        bool has(const std::string &name, bool inherit = false);

        void registerRule(CSSRule *rule);

        void clearClasses() {
            classes.clear();
        }

        bool hasPseudoClass(const std::string &name) {
            return utils::has(pseudoClasses, name);
        }

        /**
         * Remove all blocks from our list, but don't delete them (will be deleted from StyleApplier)
         */
        void deAssignRules() {
            rules.clear();
        }

        bool hasClass(const std::string &name) {
            return utils::has(classes, name);
        }

        Style2State *addPseudoClass(const std::string &name) {
            if (!utils::has(pseudoClasses, name)) {
                pseudoClasses.push_back(name);
                invalidateCache();
            }
            return this;
        }

        Style2State *addClass(const std::string &name) {
            if (!utils::has(classes, name)) {
                classes.push_back(name);
                invalidateCache();
            }
            return this;
        }

        Style2State *removePseudoClass(const std::string &name) {
            utils::remove(pseudoClasses, name);
            invalidateCache();
            return this;
        }

        Style2State *removeClass(const std::string &name) {
            utils::remove(classes, name);
            invalidateCache();
            return this;
        }

    };

    /**
     * body .classA h1:hover {}
     * body div.classA h1:hover {}
     * body div.classA:not(.classB) h1:hover {}
     * body classA:not(span.classC) h1 {}
     */
    struct CSSRuleSelectorPart {
        std::string tagName; //tagName or '*'
        std::string id;

        /**
         * * all deep children
         * > all direct children
         */
        char op = '*';

        std::vector<std::string> classes;
        std::vector<std::string> pseudoClasses;
    };

    struct CSSRuleSelector {
        unsigned long weight = 0;
        std::vector<CSSRuleSelectorPart> parts;

        //unsigned long max 4_294_967_295
        static const unsigned long WEIGHT_INLINE = 1'000'000'000;
        static const unsigned long WEIGHT_ID = 1'000'000;
        static const unsigned long WEIGHT_CLASS = 1'000;
        static const unsigned long WEIGHT_ELEMENT = 1;

        CSSRuleSelector() = default;

        std::string toString() {
            std::string str;

            for (auto &selectorPart: parts) {
                if (selectorPart.op == '>') {
                    str.append("> ");
                }

                str.append(selectorPart.tagName);

                if (!selectorPart.id.empty()) {
                    str.append("#" + selectorPart.id);
                }

                for (auto &clazz: selectorPart.classes) {
                    str.append("." + clazz);
                }
                for (auto &clazz: selectorPart.pseudoClasses) {
                    str.append(":" + clazz);
                }

                str += " ";
            }

            return str;
        }
    };

    class CSSRule {
    public:
        unsigned long orderId = 0;
        CSSRuleSelector selector;
        std::unordered_map<std::string, std::string> properties;

        CSSRule() = default;

        CSSRule(unsigned long weight) {
            selector.weight = weight;
        }

        std::string toString();

        bool active(Style2State *state) {
            if (selector.parts.empty()) return true;

            auto matches = [](CSSRuleSelectorPart &part, Style2State *state) -> bool {
                if (!part.id.empty() && (!state->id || *state->id != part.id)) {
                    return false;
                }

                if (!part.tagName.empty() && (!state->tagName || *state->tagName != part.tagName)) {
                    return false;
                }

                for (auto &clazz: part.pseudoClasses) {
                    if (!state->hasPseudoClass(clazz)) return false;
                }

                for (auto &clazz: part.classes) {
                    if (!state->hasClass(clazz)) return false;
                }

                return true;
            };

            //first read tree of all parents
            std::vector<Pesto::Style2State *> states;
            auto current = state;
            while (current) {
                states.push_back(current);
                current = current->getParent();
            }

            //now go through all parts and try to find a reason the rule doesn't apply, return false immediately.
            for (auto &part: selector.parts) {
                if (part.op == '*') {
                    //search as deep as necessary
                    bool found = false;
                    while (!states.empty()) {
                        auto next = states.back();
                        if (matches(part, next)) {
                            //we found it, so go to next selector
                            found = true;
                            states.pop_back();
                            break;
                        }
                        //not found yet, go to next state
                        states.pop_back(); //this doesnt work, WHY
                    }

                    if (!found) {
                        return false;
                    }
                }
                if (part.op == '>') {
                    //we search only direct next element
                    //if it doesnt match, we exit false
                    auto next = states.back();
                    if (!matches(part, next)) {
                        return false;
                    }

                    //found, go to next state
                    states.pop_back();
                }
            }

            return states.size() == 0;
        }

        bool has(const std::string &name) {
            return properties.find(name) != properties.end();
        }

        std::string get(const std::string &name) {
            return properties[name];
        }

        void set(const std::string &name, const std::string &value) {
            if (value == "inherit") {
                unset(name);
            } else {
                properties[name] = value;
            }
        }

        void unset(const std::string &name) {
            properties.erase(name);
        }
    };

    class Style2Property {
    protected:
        const std::string name;
        const bool inheritFromParent = false;
    public:
        Style2State *state = nullptr;

        explicit Style2Property(const std::string &name, bool inheritFromParent = false)
            : name(name), inheritFromParent(inheritFromParent) {}

        virtual bool isDefined() {
            if (state == nullptr) {
                throw std::runtime_error("No state assigned to property");
            }

            return state->has(name, inheritFromParent);
        }

        /**
         * Setting a value means we create a new block with high weight in the given state
         * and put it there.
         */
        virtual void operator=(const std::string &value) {
            if (state == nullptr) {
                throw std::runtime_error("No state assigned to property");
            }

            state->invalidateCache();
            state->getInlineRule()->set(name, value);
        }

        virtual bool operator==(const std::string &value) {
            return stringValue() == value;
        }

        virtual bool operator!=(const std::string &value) {
            return stringValue() != value;
        }

        virtual void unset() {
            if (state->hasInlineRule()) {
                state->invalidateCache();
                state->getInlineRule()->unset(name);
            }
        }

        virtual std::string stringValue() {
            if (state == nullptr) {
                throw std::runtime_error("No state assigned to property");
            }

            return state->value(name, inheritFromParent);
        }
    };

    class Style2DefaultProperty : public Style2Property {
        std::string defaultValue;
    public:
        Style2DefaultProperty(const std::string &name, bool inheritFromParent, const std::string &defaultValue)
            : Style2Property(name, inheritFromParent), defaultValue(defaultValue) {}

        using Style2Property::operator=;

        std::string stringValue() override  {
            if (state == nullptr) {
                throw std::runtime_error("No state assigned to property");
            }

            return state->value(name, inheritFromParent, defaultValue);
        }

    };

    class Style2FloatProperty : public Style2Property {
    public:
        explicit Style2FloatProperty(const char *name, bool inheritFromParent = false) :
            Style2Property(name, inheritFromParent) {}

        using Style2Property::operator=;

        void operator=(float value) {
            state->getInlineRule()->set(name, std::to_string(value).append("px"));
            state->invalidateCache();
        }

        float value(float base = 0.0f) {
            auto v = stringValue();

            if (v.empty()) {
                return 0.0f;
            }

            if (v.find('%') != std::string::npos) {
                //contains %
                return (std::stof(v) / 100) * base;
            }

            return std::stof(v);
        }
    };

    uint32_t StrToColor(const std::string &name);
    std::string ColorToString(uint32_t color);

    class Style2ColorProperty : public Style2Property {
        uint32_t color = 0;
        bool overwritten = false;
    public:
        explicit Style2ColorProperty(const char *name, bool inheritFromParent = false) :
            Style2Property(name, inheritFromParent) {}

        using Style2Property::operator=;

        void operator=(uint32_t color) {
            //todo, set property value instead of inlineRule
            overwritten = true;
            this->color = color;
        }

        uint32_t value() {
            if (overwritten) {
                return color;
            }

            return StrToColor(stringValue());
        }
    };

    class Style2BorderSideProperty : public Style2Property {
        unsigned long cacheRead = 0;
        struct {
            int width = 0;
            std::string type;
            uint32_t color = 0;
        } cache;
    public:
        explicit Style2BorderSideProperty(const char *name, bool inheritFromParent = false) :
            Style2Property(name, inheritFromParent) {}

        using Style2Property::operator=;

        void readCache() {
            if (cacheRead != state->cacheRound) {
                cacheRead = state->cacheRound;
                auto[width, type, color] = Pesto::CSSParser::parseBorder(stringValue());
                cache.width = std::stoi(width);
                cache.type = type.empty() ? "solid" : type;
                cache.color = StrToColor(color);

                if (state->has(name + "-color", false)) {
                    cache.color = StrToColor(state->value(name + "-color", false));
                }

                if (state->has(name + "-width", false)) {
                    cache.width = std::stoi(state->value(name + "-width", false));
                }
            }
        }

        void setWidth(std::string width) {
            state->getInlineRule()->set(name + "-width", width);
            state->invalidateCache();
        }

        void setWidth(int width) {
            state->getInlineRule()->set(name + "-width", std::to_string(width) + "px");
            state->invalidateCache();
        }

        void setColor(std::string color) {
            state->getInlineRule()->set(name + "-color", color);
            state->invalidateCache();
        }

        void setColor(uint32_t color) {
            state->getInlineRule()->set(name + "-color", ColorToString(color));
            state->invalidateCache();
        }

        int getWidth() {
            readCache();
            return cache.width;
        }

        uint32_t getColor() {
            readCache();
            return cache.color;
        }

        std::string getType() {
            readCache();
            return cache.type;
        }
    };


    class Style2 {
    public:
        Style2FloatProperty top{"top"};
        Style2FloatProperty left{"left"};
        Style2FloatProperty right{"right"};
        Style2FloatProperty bottom{"bottom"};

        Style2FloatProperty paddingTop{"padding-top"};
        Style2FloatProperty paddingRight{"padding-right"};
        Style2FloatProperty paddingBottom{"padding-bottom"};
        Style2FloatProperty paddingLeft{"padding-left"};

        Style2FloatProperty marginTop{"margin-top"};
        Style2FloatProperty marginRight{"margin-right"};
        Style2FloatProperty marginBottom{"margin-bottom"};
        Style2FloatProperty marginLeft{"margin-left"};

        Style2FloatProperty borderRadiusTopLeft{"border-radius-top-left"};
        Style2FloatProperty borderRadiusTopRight{"border-radius-top-right"};
        Style2FloatProperty borderRadiusBottomLeft{"border-radius-bottom-left"};
        Style2FloatProperty borderRadiusBottomRight{"border-radius-bottom-right"};

        Style2FloatProperty width{"width"};
        Style2FloatProperty height{"height"};

        Style2FloatProperty maxWidth{"max-width"};
        Style2FloatProperty maxHeight{"max-height"};

        Style2FloatProperty minWidth{"min-width"};
        Style2FloatProperty minHeight{"min-height"};

        Style2BorderSideProperty borderTop{"border-top"};
        Style2BorderSideProperty borderRight{"border-right"};
        Style2BorderSideProperty borderBottom{"border-bottom"};
        Style2BorderSideProperty borderLeft{"border-left"};

        Style2DefaultProperty display{"display", false, "block"};
        Style2DefaultProperty overflow{"overflow", false, "visible"};
        Style2DefaultProperty position{"position", false, "static"};

        Style2DefaultProperty verticalAlign{"vertical-align", true, "top"};
        Style2DefaultProperty textAlign{"text-align", true, "left"};

        Style2Property fontFamily{"font-family", true};
        Style2FloatProperty lineHeight{"line-height", true};
        Style2FloatProperty fontSize{"font-size", true};
        Style2FloatProperty fontWeight{"font-weight", true};
        Style2ColorProperty color{"color", true};

        /**
         *
         *   normal,  //remove whitespace,   auto-wrap when necessary
         *   nowrap,  //remove whitespace,   no auto-wrap (only at <br>)
         *   pre,     //preserve whitespace, no auto-wrap (only at \n)
         *   pre-line, //remove whitespace,   auto-wrap + at \n
         *   pre-wrap, //preserve whitespace, auto-wrap + at \n
         */
        Style2DefaultProperty whiteSpace{"white-space", false, "normal"};

        Style2ColorProperty backgroundColor{"background-color"};

        void linkWithState(Style2State *styleState) {
            top.state = styleState;
            left.state = styleState;
            right.state = styleState;
            bottom.state = styleState;

            paddingTop.state = styleState;
            paddingRight.state = styleState;
            paddingBottom.state = styleState;
            paddingLeft.state = styleState;

            marginTop.state = styleState;
            marginRight.state = styleState;
            marginBottom.state = styleState;
            marginLeft.state = styleState;

            borderRadiusTopLeft.state = styleState;
            borderRadiusTopRight.state = styleState;
            borderRadiusBottomLeft.state = styleState;
            borderRadiusBottomRight.state = styleState;

            width.state = styleState;
            height.state = styleState;

            maxWidth.state = styleState;
            maxHeight.state = styleState;

            minWidth.state = styleState;
            minHeight.state = styleState;

            borderTop.state = styleState;
            borderRight.state = styleState;
            borderBottom.state = styleState;
            borderLeft.state = styleState;

            display.state = styleState;
            overflow.state = styleState;
            verticalAlign.state = styleState;
            textAlign.state = styleState;
            position.state = styleState;

            fontFamily.state = styleState;
            lineHeight.state = styleState;
            fontSize.state = styleState;
            fontWeight.state = styleState;
            color.state = styleState;
            whiteSpace.state = styleState;

            backgroundColor.state = styleState;
        }

        void setProperty(Style2State &state, const std::string &name, const std::string &value) {
            state.getInlineRule()->set(name, value);
            state.invalidateCache();

            if (name == "width") width = value;
            if (name == "height") height = value;

            if (name == "top") top = value;
            if (name == "right") right = value;
            if (name == "bottom") bottom = value;
            if (name == "left") left = value;

            if (name == "padding") {
                paddingTop = value;
                paddingRight = value;
                paddingBottom = value;
                paddingLeft = value;
            }

            if (name == "padding-top") paddingTop = value;
            if (name == "padding-right") paddingRight = value;
            if (name == "padding-bottom") paddingBottom = value;
            if (name == "padding-left") paddingLeft = value;

            if (name == "border-radius") {
                borderRadiusTopLeft = value;
                borderRadiusTopRight = value;
                borderRadiusBottomLeft = value;
                borderRadiusBottomRight = value;
            }

            if (name == "border-top-left-radius") borderRadiusTopLeft = value;
            if (name == "border-top-right-radius") borderRadiusTopRight = value;
            if (name == "border-bottom-right-radius") borderRadiusBottomRight = value;
            if (name == "border-bottom-left-radius") borderRadiusBottomLeft = value;

            if (name == "margin") {
                marginTop = value;
                marginRight = value;
                marginBottom = value;
                marginLeft = value;
            }

            if (name == "margin-left") marginLeft = value;
            if (name == "margin-right") marginRight = value;
            if (name == "margin-bottom") marginBottom = value;
            if (name == "margin-top") marginTop = value;

            if (name == "display") display = value;
            if (name == "position") position = value;
            if (name == "vertical-align") verticalAlign = value;
            if (name == "text-align") textAlign = value;
            if (name == "overflow") overflow = value;
            if (name == "white-space") whiteSpace = value;

            if (name == "background") {
                //todo parse other stuff as well. gradient, repeat, bla.
                backgroundColor = value;
            }

            if (name == "background-color") backgroundColor = value;

            if (name == "border") {
                borderTop = value;
                borderRight = value;
                borderBottom = value;
                borderLeft = value;
            }

            if (name == "border-color") {
                borderTop.setColor(value);
                borderRight.setColor(value);
                borderBottom.setColor(value);
                borderLeft.setColor(value);
            }

            if (name == "border-width") {
                borderTop.setWidth(value);
                borderRight.setWidth(value);
                borderBottom.setWidth(value);
                borderLeft.setWidth(value);
            }

            if (name == "border-top-color") borderTop.setColor(value);
            if (name == "border-top-width") borderTop.setWidth(value);

            if (name == "border-right-color") borderRight.setColor(value);
            if (name == "border-right-width") borderRight.setWidth(value);

            if (name == "border-bottom-color") borderBottom.setColor(value);
            if (name == "border-bottom-width") borderBottom.setWidth(value);

            if (name == "border-left-color") borderLeft.setColor(value);
            if (name == "border-left-width") borderLeft.setWidth(value);

            if (name == "border-top") borderTop = value;
            if (name == "border-right") borderRight = value;
            if (name == "border-bottom") borderBottom = value;
            if (name == "border-left") borderLeft = value;

            if (name == "font-family") fontFamily = value;
            if (name == "font-weight") fontWeight = value;

            if (name == "font-size") fontSize = value;
            if (name == "line-height") lineHeight = value;

            if (name == "color") color = value;
        }
    };

    class Element2 {
    public:
        Style2 style;
        Style2State styleState;

        Element2() {
            style.linkWithState(&styleState);
        }
    };

    /**
     *
     * h1 {
     *    color: black;
     * }
     *
     * body div:hover h1.first {
     *    color: red;
     * }
     *  constraints1:
     *
     *
     *
     */
}

#endif //DEEPKIT_STYLE2_H
