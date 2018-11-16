//
// Created by marc on 02.06.18.
//

#ifndef DEEPKIT_ELEMENT_H
#define DEEPKIT_ELEMENT_H

#include "Base.h"
#include "core/utils.h"
#include "CSSParser.h"
#include "TextElement.h"
#include "Event.h"

#include "xml/tinyxml2.h"
#include "Node.h"
#include "Style2.h"
#include "components/RegistryController.h"

#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkParse.h>
#include <SkPictureRecorder.h>
#include <SkPictureRecorder.h>

namespace Pesto {
    class Document;

    class Window;

    struct ElementLine {
        float x = 0;
        float y = 0;
    };

    struct InnerBounds {
        //used to determine where to draw next children
        float x = 0;
        float y = 0;

        //used to determine when static children need to break-line
        float maxWidth = 0;
        float maxHeight = 0;

        //used to indicate real used space of children
        float width = 0;
        float height = 0;

        float scrollTop = 0;
        float scrollLeft = 0;
    };

    class Element : public Node {
    public:
        explicit Element();

        explicit Element(std::string tagName);

        virtual ~Element();

        std::vector<ElementLine> lines;
        InnerBounds innerBounds;
        sk_sp<SkImage> renderCache;

        RegistryController *controller = nullptr;

        Style2 style;
        Style2State styleState;

        bool isDirty = true;
        std::unordered_map<std::string, std::string> attributes;

        std::string id;
        std::string tagName;

        //used by the SelectorEngine
        unsigned long contextRound = 0;

        /**
         * Each element could have a <style> element assigned,
         * which is represented by this object.
         */
        CSSParser *cssParser = nullptr;

        virtual CSSParser *getCSSParser();

        virtual std::string path();

        std::string name() override;

        virtual std::string name(bool withAttributes);

        virtual Element *createChildren(std::string tagName);

        virtual Element *parentElement();

        virtual Element *firstChildElement();

        virtual Element *deepNextElement(Element *until = nullptr);

        virtual Element *deepNextElementSibling(Element *until = nullptr);

        virtual Element *nextElementSibling();

        virtual void appendChild(Element *child);

        void setParent(Node *parent) override;

        virtual void setCSSParser(CSSParser *parser);

        virtual void setPosition(float x, float y);

        virtual void setClass(const std::string &classes);

        virtual void setStyles(const std::string &inlineStyles);

        virtual void styleChanged(const std::string &name);

        virtual std::string getTagName();

        virtual bool hasAttribute(const std::string &name);

        virtual std::string getAttribute(const std::string &name);

        virtual bool hasClass(const std::string &className);

        virtual bool hasPseudoClass(const std::string &className);

        virtual Element *getOffsetParent();

        virtual void newLine();

        virtual Element *setText(std::string text);

        virtual void scrollX(MouseEvent x);

        virtual void scrollY(MouseEvent y);

        virtual void drawScrollBarY(SkCanvas *canvas);

        virtual void drawScrollBarX(SkCanvas *canvas);

        virtual bool isScrollBarVisibleY();

        virtual bool isScrollBarVisibleX();

        virtual Document *getDocument();

        virtual Window *getWindow();

        void addClickListener(std::function<void(MouseEvent)> callback);

        virtual void needRestyle();

        bool isElementNode() override;

        void align() override;

        void calculateLayout(SkCanvas *canvas) override;

        void drawIntoCache(SkCanvas *canvas, double diff);

        void drawFromCache(SkCanvas *canvas, double diff);

        void draw(SkCanvas *canvas, double diff) override;
    };
}


#endif //DEEPKIT_ELEMENT_H
