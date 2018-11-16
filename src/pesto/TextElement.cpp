//
// Created by marc on 04.06.18.
//

#include "TextElement.h"
#include "Element.h"
#include "TextCursor.h"
#include <boost/algorithm/string/trim.hpp>
#include <SkPaint.h>
#include <SkTypeface.h>
#include <SkSurface.h>
#include <SkImageInfo.h>

void Pesto::TextElement::draw(SkCanvas *canvas, double diff) {
    SkPaint paint;

    Element *parent = dynamic_cast<Element *>(this->parent);
    if (!parent) {
        return;
    }

    if (parent->style.color.isDefined()) {
        paint.setColor(parent->style.color.value());
    }

    if (parent->style.fontSize.isDefined()) {
        paint.setTextSize(parent->style.fontSize.value());
    }

    if (parent->style.fontFamily.isDefined()) {
        paint.setTypeface(typeface);
    }

    auto x = parent->globalPosition.x;
    auto y = parent->globalPosition.y;

    y -= parent->innerBounds.scrollTop;
    x -= parent->innerBounds.scrollLeft;

    paint.setAutohinted(true);
    paint.setAntiAlias(true);

    for (auto &&line : textLines) {
        canvas->drawString(line.str.c_str(), x + line.x, y + line.y,
                           paint);
    }
}

void Pesto::TextElement::align() {
    Element *parent = dynamic_cast<Element *>(this->parent);
    if (!parent) {
        return;
    }

    //for block elements, the maxWidth differs from regular width
    float parentWidth = parent->innerBounds.maxWidth;

    if (parent->style.display.stringValue() == "inline") {
//        //inline elements have no space left right side.
        parentWidth = parent->size.width;
        parentWidth -= parent->style.borderRight.getWidth();
        parentWidth -= parent->style.borderLeft.getWidth();
        parentWidth -= parent->style.paddingRight.value();
        parentWidth -= parent->style.paddingLeft.value();
    }

    for (auto &&textLine : textLines) {
        float moveOffset = 0;

        if (parent->style.textAlign == "right") {
            moveOffset = (parentWidth - (parent->lines[textLine.line].x - parent->innerBounds.x));
        }

        if (parent->style.textAlign == "center") {
            moveOffset = (parentWidth / 2) - ((parent->lines[textLine.line].x - parent->innerBounds.x) / 2);
        }

//        printf("Align text %s (%zu | %zu) right, moveOffset=%d, parentWidth=%d, x=%d\n",
//            name().c_str(), lineInParent, textLine.line,
//               moveOffset,
//               parentWidth,
//               parent->lines[textLine.line].x
//        );

        textLine.x += moveOffset;
        size.width = std::max(size.width, textLine.x);
    }
}

void Pesto::TextElement::calculateLayout(SkCanvas *canvas) {
//    if (!recalculationNeeded) {
//        return;
//    }

    Element *parent = dynamic_cast<Element *>(this->parent);
    if (!parent) {
        return;
    }

    SkPaint paint;
    if (!typeface && parent->style.fontFamily.isDefined()) {
        typeface = SkTypeface::MakeFromName(parent->style.fontFamily.stringValue().c_str(),
                                            SkFontStyle(300, 5, SkFontStyle::Slant::kUpright_Slant));
    }

    textLines.clear();

    //get parent's globalPosition and add border/padding to know our starting point=position
    globalPosition.x = parent->globalPosition.x;
    globalPosition.y = parent->globalPosition.y;
    size.width = 0;
    size.height = 0;

    //try to cache this
    SkPaint::FontMetrics fontMetrics;
    paint.getFontMetrics(&fontMetrics);

    if (parent->style.color.isDefined()) {
        paint.setColor(parent->style.color.value());
    }

    if (parent->style.fontSize.isDefined()) {
        paint.setTextSize(parent->style.fontSize.value());
    }

    if (parent->style.fontFamily.isDefined()) {
        paint.setTypeface(typeface);
    }

    paint.setAutohinted(true);
    paint.setAntiAlias(true);

    //read line-height from parent
    float lineHeight = 16;

    if (parent->style.lineHeight.isDefined()) {
        lineHeight = parent->style.lineHeight.value(parent->innerBounds.maxHeight);
    }

    Pesto::TextCursor c(paint, text);
    if (parent->style.whiteSpace == "pre"
        || parent->style.whiteSpace == "pre-wrap"
        || parent->style.whiteSpace == "pre-line"
        ) {
        c.setBreakOnNewLine(true);
    }

    c.setAutoWrap(true);
    if (parent->style.whiteSpace == "nowrap"
        || parent->style.whiteSpace == "pre"
        ) {
        c.setAutoWrap(false);
    }

    float maxLineWidth = parent->innerBounds.maxWidth - parent->innerBounds.x;

    bool forceAtLeastOneWord = false;
    while (!c.isEnd()) {
        auto[lineWidth, lineText] = c.nextLine(maxLineWidth, forceAtLeastOneWord);

        maxLineWidth = parent->innerBounds.maxWidth;

        float y = parent->innerBounds.y;

        if (!parent->style.verticalAlign.isDefined() ||
            parent->style.verticalAlign == "top") {
            y -= fontMetrics.fTop; //fTop is always negative
        } else if (parent->style.verticalAlign == "middle") {
            y += lineHeight / 2;
            y -= fontMetrics.fAscent / 2; //fAscent is always negative
        } else if (parent->style.verticalAlign == "bottom") {
            y += lineHeight;
            y -= fontMetrics.fBottom;
        }

        textLines.push_back(Pesto::TextLine{
            lineText,
            lineInParent + textLines.size(),
            parent->innerBounds.x,
            y
        });

        parent->innerBounds.x += lineWidth;

        size.width = std::max(size.width, parent->innerBounds.x);
        size.height = std::max(size.height, parent->innerBounds.y + lineHeight);

        parent->innerBounds.width = std::max(parent->innerBounds.width, parent->innerBounds.x);
        parent->innerBounds.height = std::max(parent->innerBounds.height, parent->innerBounds.y + lineHeight);

        //next line is an empty new line, so force at least one word.
        forceAtLeastOneWord = true;
        if (!c.isEnd()) {
            parent->newLine();
        }
    }

    //put into cache
    recalculationNeeded = false;
}

void Pesto::TextElement::setText(std::string text) {
    this->text = text;
    boost::algorithm::trim(this->text);
    needRecalculation();
    needRedraw();
}

bool Pesto::TextElement::isTextNode() {
    return true;
}

std::string Pesto::TextElement::name() {
    return "#text: " + text;
}

//void Pesto::TextElement::needRedraw() {
//    Element *parent = dynamic_cast<Element *>(this->parent);
//    if (!parent) {
//        return;
//    }
//
//    parent->needRedraw();
//}
//
//void Pesto::TextElement::needRecalculation() {
//    Element *parent = dynamic_cast<Element *>(this->parent);
//    if (!parent) {
//        return;
//    }
//
//    parent->needRecalculation();
//}

//void Pesto::TextElement::styleChanged(const std::string &name, const std::string &value) {
//    Element::styleChanged(name, value);
//
//    if (name == "font-family") {
//        typeface = NULL;
//    }
//}
