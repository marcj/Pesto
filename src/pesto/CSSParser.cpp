//
// Created by marc on 06.06.18.
//

#include "CSSParser.h"
#include "SelectorEngine.h"
#include <iostream>
#include <tuple>
#include <string>
#include <boost/algorithm/string/trim.hpp>

class Parser {
public:
    unsigned long currentPos = 0;
    unsigned long maxSize = 0;
    std::string content;

    Parser(const std::string &content) : content(content) {
        maxSize = content.size();
    }

    auto isWhiteSpace(char character) {
        return character == ' ' || character == '\n' || character == '\t';
    };

    auto inFile() {
        return currentPos < maxSize;
    };

    auto isCharacter(char character) {
        return content[currentPos] == character;
    };

    auto eatUntilNonWhitespace() {
        do {
            if (!isWhiteSpace(content[currentPos])) {
                return;
            }

            currentPos++;
        } while (currentPos < maxSize);
    };

    auto eatUntilCharacter(char character) -> std::string {
        unsigned long startPosition = currentPos;

        do {
            if (content[currentPos] == character) {
                currentPos++;
                return content.substr(startPosition, (currentPos - 1) - startPosition);
            }

            currentPos++;
        } while (currentPos < maxSize);

        return content.substr(startPosition, currentPos - startPosition);
    };

    auto eatUntilCharacters(const char *characters) -> std::tuple<char, std::string> {
        unsigned long startPosition = currentPos;

        auto isCharacterIn = [&characters](char currentChar) {
            for (auto c = characters; *c; c++) {
                if (currentChar == *c) {
                    return true;
                }
            }

            return false;
        };

        do {
            if (isCharacterIn(content[currentPos])) {
                char foundCharacter = content[currentPos];
                currentPos++;
                return std::make_tuple(foundCharacter, content.substr(startPosition, (currentPos - 1) - startPosition));
            }

            currentPos++;
        } while (currentPos < maxSize);

        return std::make_tuple((char) '\0', content.substr(startPosition, currentPos - startPosition));
    };

};

std::tuple<std::string, std::string, std::string> Pesto::CSSParser::parseBorder(const std::string &borderStyle) {
    if (borderStyle.empty()) {
        return std::make_tuple("0", "solid", "black");
    }

//    printf("ParseBorder: '%s'\n", borderStyle.c_str());

    Parser parser(borderStyle);
    parser.eatUntilNonWhitespace();

    std::string width = parser.eatUntilCharacter(' ');
    std::string type = parser.eatUntilCharacter(' ');
    std::string color = parser.eatUntilCharacter(' ');

    return std::make_tuple(width, type, color);
}

std::vector<std::tuple<std::string, std::string>> Pesto::CSSParser::parseInlineStyle(std::string inlineStyle) {
    Parser parser(inlineStyle);
    std::vector<std::tuple<std::string, std::string>> properties;

    while (parser.inFile()) {
        parser.eatUntilNonWhitespace();

        if (parser.isCharacter(';')) {
            //unexpected ;
            parser.currentPos++; //eat it and ignore
            continue;
        }

        std::string name = parser.eatUntilCharacter(':');
        boost::algorithm::trim(name);
        std::string value = parser.eatUntilCharacter(';');
        boost::algorithm::trim(value);

        parser.eatUntilNonWhitespace();

        properties.push_back(std::make_tuple(name, value));
    }

    return properties;
}

Pesto::CSSRuleSelector Pesto::CSSParser::parseSelector(const std::string &selector) {
    //split selector into its parts
    Parser selectorParser(selector);
    CSSRuleSelector ruleSelector;

    //selector: body div.classA:active h1:hover {}
    //selector: body #h1specialze:hover {}
    char currentOp = '*';

    while (selectorParser.inFile()) {
        auto selectorPart = selectorParser.eatUntilCharacter(' '); //eat first selectorPart

        if (selectorPart == ">") {
            currentOp = '>';
            continue;
        }

        Parser selectorPartParser(selectorPart);

        //selectorPart:
        // div.classA:active
        // div:hover
        // div:hover:active
        char lastConstraintMode = '\0';
        char nextConstraintMode = '\0';
        std::string constraintValue;

        CSSRuleSelectorPart part;
        part.op = currentOp;
        currentOp = '*';

        if (selectorPart == "*") {
            part.tagName = "*";
            ruleSelector.weight += CSSRuleSelector::WEIGHT_ELEMENT;
        } else {
            while (selectorPartParser.inFile()) {
                std::tie(nextConstraintMode, constraintValue) = selectorPartParser.eatUntilCharacters(".:#");
                if (!constraintValue.empty()) {
                    if (lastConstraintMode == '\0') {
                        part.tagName = constraintValue;
                        ruleSelector.weight += CSSRuleSelector::WEIGHT_ELEMENT;

                    } else if (lastConstraintMode == '#') {
                        part.id = constraintValue;
                        ruleSelector.weight += CSSRuleSelector::WEIGHT_ID;

                    } else if (lastConstraintMode == '.') {
                        part.classes.push_back(constraintValue);
                        ruleSelector.weight += CSSRuleSelector::WEIGHT_CLASS;

                    } else if (lastConstraintMode == ':') {
                        part.pseudoClasses.push_back(constraintValue);
                        ruleSelector.weight += CSSRuleSelector::WEIGHT_CLASS;
                    }
                }

                lastConstraintMode = nextConstraintMode;
            }
        }

        ruleSelector.parts.push_back(part);
    }

    return ruleSelector;
}

Pesto::CSSRule *Pesto::CSSParser::getRule(unsigned int index) {
    return &getRules()->at(index);
}

std::vector<Pesto::CSSRule> *Pesto::CSSParser::getRules() {
    if (!built) {
        Parser parser(style);
        unsigned long orderId = 0;

        auto findRule = [&]() {
            parser.eatUntilNonWhitespace();
            std::string selectors = parser.eatUntilCharacter('{');
            boost::algorithm::trim(selectors);

            if (selectors.empty()) {
                return;
            }

            if (selectors == "@font-face") {
                //todo, read file and palce in our (not-yet existing bucket).
                //SkTypeface::MakeFromFile()
                //font-family: 'FontName', Fallback, sans-serif;
                //src,
                // font-weight: normal, 600, 800, ...
                return;
            }

            Pesto::CSSRule baseCssRule;
            baseCssRule.orderId = orderId++;

            //real all properties between { and }
            while (parser.inFile() && !parser.isCharacter('}')) {
                parser.eatUntilNonWhitespace();

                if (parser.isCharacter(';')) {
                    //unexpected ;
                    parser.currentPos++; //eat it and ignore
                    continue;
                }

                std::string name = parser.eatUntilCharacter(':');
                boost::algorithm::trim(name);
                std::string value = parser.eatUntilCharacter(';');
                boost::algorithm::trim(value);

                parser.eatUntilNonWhitespace();

                baseCssRule.properties[name] = value;

                //todo, is this really the right place?
                if (name == "background") {
                    baseCssRule.properties["background-color"] = value;
                }

                if (name == "border-radius") {
                    baseCssRule.properties["border-radius-top-left"] = value;
                    baseCssRule.properties["border-radius-top-right"] = value;
                    baseCssRule.properties["border-radius-bottom-right"] = value;
                    baseCssRule.properties["border-radius-bottom-left"] = value;
                }

                if (name == "border-color") {
                    baseCssRule.properties["border-top-color"] = value;
                    baseCssRule.properties["border-right-color"] = value;
                    baseCssRule.properties["border-bottom-color"] = value;
                    baseCssRule.properties["border-left-color"] = value;
                }

                if (name == "border-width") {
                    baseCssRule.properties["border-top-width"] = value;
                    baseCssRule.properties["border-right-width"] = value;
                    baseCssRule.properties["border-bottom-width"] = value;
                    baseCssRule.properties["border-left-width"] = value;
                }

                if (name == "border") {
                    baseCssRule.properties["border-top"] = value;
                    baseCssRule.properties["border-right"] = value;
                    baseCssRule.properties["border-bottom"] = value;
                    baseCssRule.properties["border-left"] = value;
                }

                if (name == "padding") {
                    baseCssRule.properties["padding-top"] = value;
                    baseCssRule.properties["padding-right"] = value;
                    baseCssRule.properties["padding-bottom"] = value;
                    baseCssRule.properties["padding-left"] = value;
                }

                if (name == "margin") {
                    baseCssRule.properties["margin-top"] = value;
                    baseCssRule.properties["margin-right"] = value;
                    baseCssRule.properties["margin-bottom"] = value;
                    baseCssRule.properties["margin-left"] = value;
                }
            }

            parser.currentPos++; //eat }

            auto selectorSplit = utils::split(selectors, ',');
            for (auto &selector: selectorSplit) {
                boost::algorithm::trim(selector);
                if (selector.empty()) {
                    continue;
                }

                Pesto::CSSRule rule;
                rule.properties = baseCssRule.properties;
                rule.orderId = baseCssRule.orderId;
                rule.selector = CSSParser::parseSelector(selector);

                rules.push_back(rule);
            }
        };

        while (parser.inFile()) {
            findRule();
        }

        built = true;
    }

    return &rules;
}

std::string Pesto::CSSParser::getFullStyle() {
    return style;
}

std::string Pesto::CSSRule::toString() {
    std::string str;

    str.append(selector.toString());

    str.append(" {\n");
    for (auto &[name, value]: properties) {
        str.append("  " + name + ": " + value + ";\n");
    }
    str.append("}\n");

    return str;
}
