//
// Created by marc on 13.06.18.
//

#ifndef DEEPKIT_NODE_H
#define DEEPKIT_NODE_H

#include <SkCanvas.h>
#include "Base.h"

namespace Pesto {
    class Document;

    class Node {
    public:
        Node *parent = nullptr;
        std::vector<Node *> children;
        Node *prev = nullptr;
        Node *next = nullptr;

        Node *firstChild = nullptr;
        Node *lastChild = nullptr;

        unsigned long lineInParent = 0; //indicates in which line this element lives from the parent
        unsigned int depth = 0;

        //x,y position to x-y of window
        XYPosition globalPosition;

        bool cacheRender = false;
        Size size;

        virtual ~Node();

        virtual bool hit(float x, float y);

        virtual bool intersects(float x, float y, float width, float height);

        virtual void updateDepth();

        virtual void setParent(Node *parent);

        virtual void addChildren(Node *child);

        virtual void removeChild(Node *child);

        virtual void destroyChildren();

        virtual bool isElementNode();

        virtual bool isTextNode();

        virtual bool isCacheRenderingEnabled();

        virtual Node *deepNext(Node *until = nullptr);

        virtual Node *deepNextSibling(Node *until = nullptr);

        virtual bool hasChild();

        bool recalculationNeeded = true;

        //recalculation means we call the calculateLayout() method in next frame (before draw)
        virtual void needRecalculation();

        //Redraw means we call the draw() method in next frame
        bool redrawNeeded = true;

        virtual void needRedraw();

        virtual std::string toString();

        virtual std::string name() = 0;

        virtual void align() = 0;

        virtual void calculateLayout(SkCanvas *canvas) = 0;

        virtual void draw(SkCanvas *canvas, double diff) = 0;
    };
}


#endif //DEEPKIT_NODE_H
