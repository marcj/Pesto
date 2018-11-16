//
// Created by marc on 06.06.18.
//

#ifndef DEEPKIT_CSSPARSER_H
#define DEEPKIT_CSSPARSER_H

#include <string>
#include <map>
#include <vector>
#include <tuple>
#include <map>

namespace Pesto {
    class Element;
    class CSSRule;

    struct CSSRuleSelector;

    class CSSParser {
        bool built = false;
        std::string style;
    public:
        std::vector<CSSRule> rules;

        explicit CSSParser(const std::string &style) : style(style) {};

        static std::tuple<std::string, std::string, std::string> parseBorder(const std::string &borderStyle);

        static std::vector<std::tuple<std::string, std::string>> parseInlineStyle(std::string inlineStyle);

        static CSSRuleSelector parseSelector(const std::string &selector);

//        static std::vector<CSSRule> parse(const std::string &style);
//        static std::vector<CSSRule> parse(const std::string &style);


        Pesto::CSSRule *getRule(unsigned int index);

        std::vector<CSSRule> *getRules();

        std::string getFullStyle();

//        void getRulesForElement(const Pesto::Element &element);
    };

}

#endif //DEEPKIT_CSSPARSER_H
