//
// Created by marc on 01.06.18.
//

#ifndef DEEPKIT_SCREEN_H
#define DEEPKIT_SCREEN_H

#include "Base.h"
#include "SkCanvas.h"
#include "Element.h"

namespace Pesto {
    class Window;

    class Document : public Element, public LifeTimeEventDispatcher {
    public:
        Window *window = nullptr;

        explicit Document(Window *window);

        Element *createElement(std::string tagName);

        std::vector<Pesto::Element *> query(const std::string &selector);

        Element *queryOne(const std::string &selector);

        Element *fromXML(const std::string &xml);

        void updateDepth() override;

        //Restyle means we assign .rules from CSS rules to styleState.
        //necessary when its class changes or new object is inserted into the DOM
        bool restyleNeeded = true;

        void restyle();

        Document *getDocument() override;

        Window *getWindow() override;

        virtual ~Document();

        void tick(SkCanvas *canvas, double diff);

        bool isCacheRenderingEnabled() override;
    };
}


#endif //DEEPKIT_SCREEN_H
