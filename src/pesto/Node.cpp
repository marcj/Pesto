//
// Created by marc on 13.06.18.
//

#include "Node.h"

void Pesto::Node::addChildren(Pesto::Node *child) {
    child->setParent(this);
}

void Pesto::Node::setParent(Pesto::Node *parent) {
    if (this->parent) {
        this->parent->removeChild(this);
    }

    this->parent = parent;

    if (parent) {
        //add it as direct children
        parent->children.push_back(this);

        //update lastChild of parent
        if (parent->lastChild) {
            parent->lastChild->next = this;
            prev = parent->lastChild;
        }

        //set as firstChild, if no child exists already
        if (!parent->firstChild) {
            parent->firstChild = this;
        }

        parent->lastChild = this;
        parent->needRecalculation();
        parent->needRedraw();
    }

    updateDepth();
    needRecalculation();
    needRedraw();
}

void Pesto::Node::needRecalculation() {
//    for (Node *node = this; node; node = node->deepNext(this)) {
//        node->recalculationNeeded = true;
//    }
    recalculationNeeded = true;
//
//    //necessary? Yes, since children wouldn't get calculateLayout() when parent is not marked as recalculationNeeded=true
    for (auto current = this; current; current = current->parent) {
        if (!current->parent) {
            //we reached document
            current->recalculationNeeded = true;
        }
    }
}

void Pesto::Node::needRedraw() {
    for (auto current = this; current; current = current->parent) {
        if (current->isCacheRenderingEnabled()) {
            current->redrawNeeded = true;
            break;
        }
    }
}

void Pesto::Node::removeChild(Pesto::Node *child) {
    utils::remove(children, child);

    if (child->prev && child->next) {
        child->prev->next = child->next;
        child->next->prev = child->prev;
    }

    if (child->prev && !child->next) {
        child->prev->next = nullptr;
    }

    if (!child->prev && child->next) {
        child->next->prev = nullptr;
    }

    if (firstChild == child) {
        firstChild = child->next;
    }

    if (lastChild == child) {
        lastChild = child->prev;
    }

    child->prev = nullptr;
    child->next = nullptr;
    child->parent = nullptr;
}

std::string Pesto::Node::toString() {
    std::string str;

    for (Pesto::Node *item = this; item; item = item->deepNext()) {
        str.append(std::string(item->depth * 2, ' '));
        str.append(item->name());
        str.append("\n");
    }

    return str;
}


bool Pesto::Node::hasChild() {
    return children.empty();
}

bool Pesto::Node::isTextNode() {
    return false;
}

Pesto::Node *Pesto::Node::deepNext(Node *until) {
    if (firstChild) {
        return firstChild;
    }

    //no children anymore, so go next sibling, or upwards
    return deepNextSibling(until);
}

Pesto::Node *Pesto::Node::deepNextSibling(Node *until) {
    if (next) {
        return next;
    }

    //no sibling, got to parent's next
    if (parent) {
        auto currentParent = parent;
        while (currentParent) {
            if (until == currentParent) {
                return nullptr;
            }

            if (currentParent->next) {
                return currentParent->next;
            }

            currentParent = currentParent->parent;
        }
    }

    return nullptr;
}

bool Pesto::Node::isElementNode() {
    return false;
}

void Pesto::Node::updateDepth() {
    if (parent) {
        depth = parent->depth + 1;
    } else {
        depth = 1; //0 has always document
    }

    for (auto &child: children) {
        child->updateDepth();
    }
}

void Pesto::Node::destroyChildren() {
    auto copy = children;
    for (Node *item : copy) {
        delete item;
    }

    children.clear();
}

Pesto::Node::~Node() {
    destroyChildren();
}

bool Pesto::Node::hit(float x, float y) {
    return globalPosition.x < x
           && (globalPosition.x + size.width) >= x
           && globalPosition.y < y
           && (globalPosition.y + size.height) >= y;
}

bool Pesto::Node::intersects(float x, float y, float width, float height) {
    return globalPosition.x < x + width
           && globalPosition.x + size.width > x
           && globalPosition.y > y + height
           && globalPosition.y + size.height < y;
}

bool Pesto::Node::isCacheRenderingEnabled() {
    return cacheRender;
}