//
// Created by marc on 04.06.18.
//

#ifndef DEEPKIT_DOMREADER_H
#define DEEPKIT_DOMREADER_H

#include "Element.h"
#include "TextElement.h"
#include <string>

namespace Pesto {
    class DomReader {

    public:
        DomReader();

//        virtual Pesto::Element &fromXML(Element &parent, char *xml);

        virtual void applyDomProperties(Pesto::Element &element, tinyxml2::XMLElement &domElement);
        virtual void applyControllers(Pesto::Element &element);

        virtual void insertDomChildren(Pesto::Element &element, tinyxml2::XMLNode &domElement);

        static Pesto::Element *populate(const std::string &xml, Pesto::Element *parent = nullptr);
    };
}


#endif //DEEPKIT_DOMREADER_H
