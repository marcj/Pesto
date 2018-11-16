//
// Created by marc on 02.06.18.
//

#include "Element.h"

#include "Document.h"
#include "Window.h"
#include "SkPath.h"
#include "SkCornerPathEffect.h"

#include <algorithm>
#include <SkSurface.h>
#include <SkImageInfo.h>

void Pesto::Element::setParent(Pesto::Node *parent) {
    Node::setParent(parent);

    if (auto element = dynamic_cast<Pesto::Element *>(parent)) {
        element->needRestyle();
        element->needRecalculation();
        styleState.setParent(&element->styleState);
    }
}

std::string Pesto::Element::path() {
    std::string path;
    for (Element *i = this; i; i = i->parentElement()) {
        if (path.empty()) {
            path = i->name(false);
        } else {
            path = i->name(false) + " > " + path;
        }
    }

    return path;
}

std::string Pesto::Element::name(bool withAttributes) {
    std::string name(tagName);

    if (!id.empty()) {
        tagName + "#" + id;
    }

    if (hasAttribute("class")) {
        tagName + "." + getAttribute("class");
    }

    if (withAttributes) {
        for (auto &&[k, v]: attributes) {
            if (k == "id") continue;
            if (k == "class") continue;

            name.append(" " + k + "=\"" + v + "\"");
        }
    }

    return name;
}

std::string Pesto::Element::name() {
    return name(true);
}

Pesto::Element::~Element() {
    if (parent) {
        parent->removeChild(this);
    }

    delete cssParser;
}

void Pesto::Element::drawIntoCache(SkCanvas *canvas, double diff) {
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(canvas->getGrContext(), SkBudgeted::kNo,
                                                           canvas->imageInfo());
    auto cacheCanvas = surface->getCanvas();
    cacheCanvas->clear(SK_ColorTRANSPARENT);

    for (auto &&item : children) {
        if (!item->isCacheRenderingEnabled()) {
            item->draw(cacheCanvas, diff);
        }
    }

//    printf("drawIntoCache %s\n", path().c_str());
    surface->getCanvas()->flush();
    renderCache = surface->makeImageSnapshot();
    redrawNeeded = false;
}

void Pesto::Element::drawFromCache(SkCanvas *canvas, double diff) {
    if (!renderCache || redrawNeeded) {
        drawIntoCache(canvas, diff);
    }

    //0,0 because surface has 100% size of document
    canvas->drawImage(renderCache, 0, 0);
}

void Pesto::Element::draw(SkCanvas *canvas, double diff) {
    //todo, store how many redraws this element got, so we can debug later in dev inspector (yet to come)

    int rootX = (int) globalPosition.x;
    int rootY = (int) globalPosition.y;
//    int rootX = 0;
//    int rootY = 0;

    SkRRect outer;
    SkRect rectOuter = SkRect::MakeXYWH(rootX, rootY, size.width, size.height);
    outer.setNinePatch(rectOuter,
                       style.borderRadiusTopLeft.value(),
                       style.borderRadiusTopRight.value(),
                       style.borderRadiusBottomRight.value(),
                       style.borderRadiusBottomLeft.value()
    );

//        path.setFillType(SkPath::kInverseWinding_FillType);
    canvas->save();
    canvas->clipRRect(outer, true);

    if (style.backgroundColor.isDefined()) {
        SkPaint paint;
        paint.setColor(style.backgroundColor.value());
        canvas->drawRect(SkRect::MakeXYWH(rootX, rootY, size.width, size.height), paint);
    }

    if (style.borderTop.getWidth() || style.borderRight.getWidth() || style.borderBottom.getWidth() ||
        style.borderLeft.getWidth()) {
        SkPaint paint;
        paint.setAntiAlias(true);

        SkRect rectInner = SkRect::MakeXYWH(rootX + style.borderLeft.getWidth(),
                                            rootY + style.borderTop.getWidth(),
                                            size.width - style.borderLeft.getWidth() - style.borderRight.getWidth(),
                                            size.height - style.borderTop.getWidth() - style.borderBottom.getWidth()
        );

        SkRRect inner;
        inner.setNinePatch(rectInner, style.borderRadiusTopLeft.value(), style.borderRadiusTopRight.value(),
                           style.borderRadiusBottomRight.value(), style.borderRadiusBottomLeft.value());

        //the radius from the inner Rect should start when border size is consumed
        SkVector radii[4];
        radii[SkRRect::kUpperLeft_Corner] = {
            style.borderRadiusTopLeft.value() - style.borderLeft.getWidth(),
            style.borderRadiusTopLeft.value() - style.borderTop.getWidth(),
        };
        radii[SkRRect::kUpperRight_Corner] = {
            style.borderRadiusTopRight.value() - style.borderRight.getWidth(),
            style.borderRadiusTopRight.value() - style.borderTop.getWidth(),
        };
        radii[SkRRect::kLowerRight_Corner] = {
            style.borderRadiusBottomRight.value() - style.borderRight.getWidth(),
            style.borderRadiusBottomRight.value() - style.borderBottom.getWidth(),
        };
        radii[SkRRect::kLowerLeft_Corner] = {
            style.borderRadiusBottomLeft.value() - style.borderLeft.getWidth(),
            style.borderRadiusBottomLeft.value() - style.borderBottom.getWidth(),
        };
        inner.setRectRadii(rectInner, radii);

        bool allSameColor = true;

        if (style.borderTop.getWidth() && style.borderRight.getWidth()) {
            //both have width, so check if color is same
            allSameColor = style.borderTop.getColor() == style.borderRight.getColor();
        }

        if (allSameColor && style.borderRight.getWidth() && style.borderBottom.getWidth()) {
            //both have width, so check if color is same
            allSameColor = style.borderRight.getColor() == style.borderBottom.getColor();
        }

        if (allSameColor && style.borderBottom.getWidth() && style.borderLeft.getWidth()) {
            //both have width, so check if color is same
            allSameColor = style.borderBottom.getColor() == style.borderLeft.getColor();
        }

        if (allSameColor && style.borderBottom.getWidth() && style.borderLeft.getWidth()) {
            //both have width, so check if color is same
            allSameColor = style.borderBottom.getColor() == style.borderLeft.getColor();
        }

        if (allSameColor && style.borderLeft.getWidth() && style.borderTop.getWidth()) {
            //both have width, so check if color is same
            allSameColor = style.borderLeft.getColor() == style.borderTop.getColor();
        }

        if (allSameColor && style.borderLeft.getWidth() && style.borderRight.getWidth()) {
            //both have width, so check if color is same
            allSameColor = style.borderLeft.getColor() == style.borderRight.getColor();
        }

        if (allSameColor && style.borderBottom.getWidth() && style.borderTop.getWidth()) {
            //both have width, so check if color is same
            allSameColor = style.borderBottom.getColor() == style.borderTop.getColor();
        }

        if (allSameColor) {
            //all colors the same, so draw just one big rectangle
            if (style.borderTop.getWidth()) paint.setColor(style.borderTop.getColor());
            if (style.borderRight.getWidth()) paint.setColor(style.borderRight.getColor());
            if (style.borderBottom.getWidth()) paint.setColor(style.borderBottom.getColor());
            if (style.borderLeft.getWidth()) paint.setColor(style.borderLeft.getColor());

            canvas->drawDRRect(outer, inner, paint);
        } else {

            //we have different colors, so draw each side with a clip
            if (style.borderTop.getWidth()) {
                paint.setColor(style.borderTop.getColor());

                SkRect clipRect = SkRect::MakeXYWH(
                    rootX,
                    rootY,
                    size.width,
                    style.borderTop.getWidth() +
                    std::max(style.borderRadiusTopLeft.value(), style.borderRadiusTopRight.value())

                );

                canvas->save();
                canvas->clipRect(clipRect, true);
                canvas->drawDRRect(outer, inner, paint);
                canvas->restore();

//                SkPaint paint2;
//                paint2.setColor(0xff00ff00);
//                canvas->drawRect(clipRect, paint2);
            }

            if (style.borderBottom.getWidth()) {
                paint.setColor(style.borderBottom.getColor());

                float maxHeight = style.borderBottom.getWidth() +
                                  std::max(style.borderRadiusBottomLeft.value(),
                                           style.borderRadiusBottomRight.value());
                SkRect clipRect = SkRect::MakeXYWH(
                    rootX,
                    rootY + size.height - maxHeight,
                    size.width,
                    maxHeight
                );

                canvas->save();
                canvas->clipRect(clipRect, true);
                canvas->drawDRRect(outer, inner, paint);
                canvas->restore();
            }

            if (style.borderRight.getWidth()) {
                paint.setColor(style.borderRight.getColor());

                float maxWidth = style.borderRight.getWidth();
                SkRect clipRect = SkRect::MakeXYWH(
                    rootX + size.width - maxWidth,
                    rootY + style.borderTop.getWidth(),
                    maxWidth,
                    size.height - style.borderTop.getWidth() - style.borderBottom.getWidth()
                );

                canvas->save();
                canvas->clipRect(clipRect, true);
                canvas->drawDRRect(outer, inner, paint);
                canvas->restore();

//                SkPaint paint2;
//                paint2.setColor(0xff00ff00);
//                canvas->drawRect(clipRect, paint2);
            }

            if (style.borderLeft.getWidth()) {
                paint.setColor(style.borderLeft.getColor());

                float maxWidth = style.borderLeft.getWidth();
                SkRect clipRect = SkRect::MakeXYWH(
                    rootX,
                    rootY + style.borderTop.getWidth(),
                    maxWidth,
                    size.height - style.borderTop.getWidth() - style.borderBottom.getWidth()
                );

                canvas->save();
                canvas->clipRect(clipRect, true);
                canvas->drawDRRect(outer, inner, paint);
                canvas->restore();
            }

        }

    }

    if (style.overflow == "visible") {
        //when we display the inner stuff out of our bound box, we stop here already our clipping
        canvas->restore();
    }

    //cache stuff without children's drawings.
    //todo, is this really faster? Probably only for text.

    for (auto &&item : children) {
        if (!item->isCacheRenderingEnabled()) {
            item->draw(canvas, diff);
        }
    }

    if (style.overflow != "visible") {
        //we strip all stuff inside, but need to stop it now.
        canvas->restore();
    }

    if (isScrollBarVisibleY()) {
        drawScrollBarY(canvas);
    }

    if (isScrollBarVisibleX()) {
        drawScrollBarX(canvas);
    }
}

void Pesto::Element::scrollY(MouseEvent e) {

    if (innerBounds.height >= size.height) {
        float max = innerBounds.height - size.height;

        if (innerBounds.scrollTop < max && e.wheelY > 0) {
            innerBounds.scrollTop += e.wheelY;
            //we consumed it, so don't trigger scrollY in parents
            e.stopPropagation();

            //todo, for the moment, layoutCalculation includes scroll positions (so for children), we should move that to draw only
            //to save calculations when scrolling
            needRecalculation();
            needRedraw();
//            printf("[%s] scrollY 1: %d, height=%.2f, bounds.height=%.2f, max=%.2f, now=%.2f\n", path().c_str(), e.wheelY, size.height, innerBounds.height, max, innerBounds.scrollTop);
        }

        if (innerBounds.scrollTop > 0 && e.wheelY < 0) {
            innerBounds.scrollTop += e.wheelY;
            //we consumed it, so don't trigger scrollY in parents
            e.stopPropagation();

            //todo, for the moment, layoutCalculation includes scroll positions (so for children), we should move that to draw only
            //to save calculations when scrolling
            needRecalculation();
            needRedraw();
//            printf("[%s] scrollY 2: %d, height=%.2f, bounds.height=%.2f, max=%.2f, now=%.2f\n", path().c_str(), e.wheelY, size.height, innerBounds.height, max, innerBounds.scrollTop);
        }


        if (innerBounds.scrollTop > max) {
            innerBounds.scrollTop = max;
        }

        if (innerBounds.scrollTop < 0) {
            innerBounds.scrollTop = 0;
        }
    }
}

void Pesto::Element::scrollX(MouseEvent e) {

}

void Pesto::Element::drawScrollBarY(SkCanvas *canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    float barWidth = 15;
    float barSpacing = 3;
    float barRadius = 5;

    float x = globalPosition.x;
    float y = globalPosition.y;

    x += style.borderRight.getWidth();
    y += style.borderTop.getWidth();

    float height = size.height;
    height -= style.borderTop.getWidth();
    height -= style.borderBottom.getWidth();

    float width = size.width;
    width -= style.borderLeft.getWidth();
    width -= style.borderRight.getWidth();

    //draw bg
    paint.setColor(0xffdddddd);
    canvas->drawRect(
        SkRect::MakeXYWH(x + width - barWidth, y, barWidth, height), paint);

    float maxKnobHeight = height - (barSpacing * 2);

    float knobHeight = 0;

    knobHeight = maxKnobHeight * (size.height / innerBounds.height);
    float scrollTop = innerBounds.scrollTop * (size.height / innerBounds.height);

    knobHeight = std::min(knobHeight, maxKnobHeight);
//    printf("%s: Draw scrollbar knob: %f/%f => %f => %f\n", name().c_str(), size.height, innerBounds.height, size.height / innerBounds.height, knobHeight);

    //draw knob
    paint.setColor(0xffaaaaaa);
    canvas->drawRoundRect(
        SkRect::MakeXYWH((x + width) - barWidth + barSpacing, y + barSpacing + scrollTop, barWidth - (barSpacing * 2),
                         knobHeight),
        barRadius, barRadius, paint);
}

void Pesto::Element::drawScrollBarX(SkCanvas *canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    float barHeight = 15;
    float barSpacing = 3;
    float barRadius = 5;

    float x = globalPosition.x;
    float y = globalPosition.y;

    x += style.borderLeft.getWidth();
    y += style.borderTop.getWidth();

    float height = size.height;
    height -= style.borderTop.getWidth();
    height -= style.borderBottom.getWidth();

    float width = size.width;
    width -= style.borderLeft.getWidth();
    width -= style.borderRight.getWidth();

    if (isScrollBarVisibleX()) {
        //other scrollbar is visible as well, so we decrease height a bit
        width -= barHeight;
    }

    //draw bg
    paint.setColor(0xffdddddd);
    canvas->drawRect(SkRect::MakeXYWH(x, y + height - barHeight, width, barHeight), paint);

    float maxKnobWidth = width - (barSpacing * 2);
    float knobWidth = 0;

    knobWidth = maxKnobWidth * (size.width / innerBounds.width);
    float scrollLeft = innerBounds.scrollLeft * (size.width / innerBounds.width);

    knobWidth = std::min(knobWidth, maxKnobWidth);

    //draw knob
    paint.setColor(0xffaaaaaa);
    canvas->drawRoundRect(
        SkRect::MakeXYWH(x + barSpacing + scrollLeft, (y + height) - barHeight + barSpacing, knobWidth,
                         barHeight - (barSpacing * 2)),
        barRadius, barRadius, paint);
}


void Pesto::Element::calculateLayout(SkCanvas *canvas) {
    Element *parent = dynamic_cast<Element *>(this->parent);
    if (!parent) {
        return;
    }

//    printf("calculateLayout %s\n", path().c_str());

    //scenario one
    //-> position: absolute. with is set or dynamically set based on children or based on left<->right.

    //scenario two
    //-> position: static. with is set or dynamically set based on children

    //when size is dynamically set:
    //-> when direct children are drawn, they query the parent's maximum size

    //when position absolute:
    //-> children will get the offset-parent (first parent that has position: absolute set)

    //when children are static
    //-> they position itself automatically and modify this innerBounds struct, so next children know where to position
    globalPosition.x = 0;
    globalPosition.y = 0;
    auto *offsetParent = getOffsetParent();

    lineInParent = parent->lines.size();

    lines.clear();

    Size oldSize = size;

    size.width = -1;
    size.height = -1;
    innerBounds.maxWidth = -1;
    innerBounds.maxHeight = -1;

    //maxWidth and width are different things. Make sure this is clear here.
    if (style.width.isDefined()) size.width = style.width.value(parent->innerBounds.maxWidth);
    if (style.height.isDefined()) size.height = style.height.value(parent->innerBounds.maxHeight);

    if (style.position == "absolute") {

        globalPosition.x = offsetParent->globalPosition.x;
        globalPosition.y = offsetParent->globalPosition.y;

        if (style.top.isDefined()) globalPosition.y += style.top.value(parent->size.height);
        if (style.left.isDefined()) globalPosition.x += style.left.value(parent->size.width);

        globalPosition.y += offsetParent->style.borderTop.getWidth();
        globalPosition.x += offsetParent->style.borderLeft.getWidth();

        if (!style.height.isDefined() && (style.top.isDefined() && style.bottom.isDefined())) {
            //height is automatically calculated based on parent
            size.height = offsetParent->size.height;
            size.height -= offsetParent->style.borderTop.getWidth();
            size.height -= offsetParent->style.borderBottom.getWidth();
            size.height -= style.top.value(parent->size.height); //we subtract the top offset
            size.height -= style.bottom.value(parent->size.height); //we subtract the bottom offset
        }

        if (!style.width.isDefined() && (style.left.isDefined() && style.right.isDefined())) {
            //width is automatically calculated based on parent
            size.width = offsetParent->size.width;
            size.width -= offsetParent->style.borderLeft.getWidth();
            size.width -= offsetParent->style.borderRight.getWidth();
            size.width -= style.left.value(parent->size.width); //we subtract the left offset
            size.width -= style.right.value(parent->size.width); //we subtract the right offset
        }

    } else {
        //our position is based on our siblings and parent's position. So its `position: static/relative`
        globalPosition.x = parent->globalPosition.x;
        globalPosition.y = parent->globalPosition.y;

        globalPosition.y -= parent->innerBounds.scrollTop;
        globalPosition.x -= parent->innerBounds.scrollLeft;

        //todo, implement `position: relative` offsets

        if (style.display == "block") {
            //we break the line, so our y will be the height of the parent's innerBounds.height
            parent->newLine();
            lineInParent++;

            if (tagName != "br") {
                if (parent->style.display != "block" && parent->innerBounds.maxWidth <= 0) {
                    throw std::runtime_error(
                        "You put a block element into a non-block element without providing a width. "
                        "Not allowed as we can not calculate the width");
                }

                //we're a static block element and have no width yet, so use parent's max width
                if (size.width <= 0 && parent->innerBounds.maxWidth) {
                    size.width = parent->innerBounds.maxWidth;
                }

                //block elements decrease the size when we have a margin
                size.width -= style.marginLeft.value();
                size.width -= style.marginRight.value();
            }

            //not for the height, right?
            //if (size.height != -1 && style.marginTop) size.height -= *style.marginTop;
            //if (size.height != -1 && style.marginBottom) size.height -= *style.marginBottom;
        }

        //we offset our position since we have a margin
        globalPosition.y += style.marginTop.value();
        globalPosition.x += style.marginLeft.value();

        //add to position parent's innerBounds.x, innerBounds.y.
        globalPosition.x += parent->innerBounds.x;
        globalPosition.y += parent->innerBounds.y;


        //todo, check if we need to be drawn (is visible in parent->innerBounds and parent doesn't hide overflows via hidden or scroll or auto)

    }

    //before we calculate children, we need to prepare innerBounds
    innerBounds.x = 0;
    innerBounds.y = 0;

    //we increase our innerBounds (since they are relative to globalPosition)
    innerBounds.x += style.paddingLeft.value();
    innerBounds.x += style.borderLeft.getWidth();

    innerBounds.y += style.paddingTop.value();
    innerBounds.y += style.borderTop.getWidth();

    if (size.width > 0) {
        innerBounds.maxWidth = size.width;
    } else {
        //we don't have a size, but need to have a maxWidth. Get it from the parent
        //maxWidth is primarily used by TextElement, to see how
        //wide the text can be and child block elements
        innerBounds.maxWidth = parent->innerBounds.maxWidth;
    }

    innerBounds.maxWidth -= style.borderLeft.getWidth();
    innerBounds.maxWidth -= style.borderRight.getWidth();

    innerBounds.maxWidth -= style.paddingLeft.value();
    innerBounds.maxWidth -= style.paddingRight.value();


    if (size.height > 0) {
        innerBounds.maxHeight = size.height;
    } else {
        //we don't have a size
        innerBounds.maxHeight = parent->innerBounds.maxHeight;
    }

    //same for height
    innerBounds.maxHeight -= style.borderTop.getWidth();
    innerBounds.maxHeight -= style.borderBottom.getWidth();

    innerBounds.maxHeight -= style.paddingTop.value();
    innerBounds.maxHeight -= style.paddingBottom.value();

    //this is set by the children, so we know how much placed they took
    //used for scrollbars, and used as our width when we have nothing specified
    innerBounds.height = 0;
    innerBounds.width = 0;

//    //when we have padding or border, we have already a size for the innerBounds
    innerBounds.width += style.paddingLeft.value();
    innerBounds.width += style.borderLeft.getWidth();
    innerBounds.height += style.paddingTop.value();
    innerBounds.height += style.borderTop.getWidth();

    //CALCULATE CHILDREN
    //when want to allow `position: absolute` to size them automatically on our
    //size, we need to have here two loops. One of direct children with `position: static` and then
    //then `position: absolute` one, since we know our size definitely when static' were handled.
    for (auto &&item : children) {
        if (item->isElementNode()) {
            auto element = dynamic_cast<Pesto::Element *>(item);
            if (element->style.position != "absolute") {
                //absolute comes after static/relative, so we know dimensions (left/right) of absolute elements
                item->lineInParent = lines.size();
                item->calculateLayout(canvas);
            }
        } else {
            //calc text right away.
            item->lineInParent = lines.size();
            item->calculateLayout(canvas);
        }
    }

    //when we have padding or border, we need to further extend innerBound's size.
    innerBounds.width += style.paddingRight.value();
    innerBounds.width += style.borderRight.getWidth();
    innerBounds.height += style.paddingBottom.value();
    innerBounds.height += style.borderBottom.getWidth();

    //when we have dynamic size, we set it now based on the innerBounds
    //so the decorations (border&background) and scrollbar know where to draw

    //we have no explicit size set, so use it from the innerBounds
    if (size.width == -1) {
        size.width = ceil(innerBounds.width);
    }

    if (size.height == -1) {
        size.height = ceil(innerBounds.height);
    }

    //we have now final size, so position: absolute can position them correctly.

    for (auto &&item : children) {
        if (item->isElementNode()) {
            auto element = dynamic_cast<Pesto::Element *>(item);
            if (element->style.position == "absolute") {
                //now absolute elements
                item->calculateLayout(canvas);
            }
        }
    }

    //todo, if we have maxHeight, or maxWidth, here is the place to set it.

    //since we know now our actual size, we calculate the final position
    //bottom aligned
    if (!style.top.isDefined() && style.bottom.isDefined()) {
        //we calculate y automatically
        float offset = offsetParent->size.height - size.height - style.bottom.value();
        //we need to change our globalPosition and all children as well :O
        for (Pesto::Node *item = this; item; item = item->deepNext(this)) {
            item->globalPosition.y += offset;
        }
    }

    //right positioned relative to offsetParent
    if (!style.left.isDefined() && style.right.isDefined()) {
        //we calculate x automatically
        float offset = offsetParent->size.width - size.width - style.right.value();
        if (offset) {
            //we need to change our globalPosition and all children as well :O
            for (Pesto::Node *item = this; item; item = item->deepNext(this)) {
                item->globalPosition.x += offset;
            }
        }
    }

    //update parent's innerBounds
    if (style.position == "static" || style.position == "relative") {
        //we were drawn by dynamic position,
        //so increase parent's innerBounds

        float tookHeight = size.height;
        float tookWidth = size.width;

        tookHeight += style.marginTop.value();
        tookHeight += style.marginBottom.value();

        //when we have a margin, we offset parent's innerBounds a bit for the next element

        tookWidth += style.marginLeft.value();
        tookWidth += style.marginRight.value();

        parent->innerBounds.width = std::max(parent->innerBounds.width, parent->innerBounds.x + tookWidth);
        parent->innerBounds.height = std::max(parent->innerBounds.height, parent->innerBounds.y + tookHeight);

        parent->innerBounds.x += tookWidth;

        if (style.display == "block") {
            //we are a block element, which means we extend the parent's innerBounds.y by our height
            //so other siblings will be drawn directly below us
            //todo, make it that it will be detected automatically by the next children
            parent->newLine();
        } else {
            //for inline elements we increase only x
            //once the children breaks the line, the children will increase y on its own
            //maybe detect auto newline?
        }
    }

    //we need to make sure, we got all line information, important for the alignment
    newLine();

    //align stuff
    if (style.textAlign != "left") {
        for (auto &&item: children) {
            item->align();
        }
    }

//    if (!parent->recalculationNeeded) {
//        //parent hasn't been calculated in this round,
//        //so trigger it manually (which triggers parent's parent as well if necessary etc) if necessary:
//        //"necessary" when our dimension changed
//        //todo, don't forget box-shadow? and overflow: visible
//        bool dimensionChanged = oldSize.width != size.width || oldSize.height != size.height;
//        if (dimensionChanged) {
//            parent->calculateLayout(canvas);
//        }
//
//        //redraw if necessary when dimension changed (due to background, border, text alignment, etc)
//        //but also when styling changed (but style changes already write into reDrawNeeded directly in styleChanged)
//        if (dimensionChanged) {
//            redrawNeeded = true;
//        }
//    }

//    //important to keep it here, so our children know whether we were in layout calculation as well
//    recalculationNeeded = false;
}

void Pesto::Element::align() {
    Element *parent = dynamic_cast<Element *>(this->parent);
    if (!parent) {
        return;
    }

    float parentWidth = parent->innerBounds.maxWidth;

    if (parent->style.display == "inline") {
        //inline elements have no space left at right side.
        parentWidth -= parent->style.borderRight.getWidth();
        parentWidth -= parent->style.borderLeft.getWidth();
        parentWidth -= parent->style.paddingRight.value();
        parentWidth -= parent->style.paddingLeft.value();
    }

    float moveOffset = 0;

    if (parent->style.textAlign == "right") {
        moveOffset = (parentWidth - (parent->lines[lineInParent].x - parent->innerBounds.x));
    }
    if (parent->style.textAlign == "center") {
        moveOffset = (parentWidth / 2) - ((parent->lines[lineInParent].x - parent->innerBounds.x) / 2);
    }

    globalPosition.x += moveOffset;

//    printf("Align %s (%zu) right, spaceLeftAtTheRightSide=%d, parent->innerBounds.maxWidth=%d, x=%d\n",
//        name().c_str(), lineInParent,
//        spaceLeftAtTheRightSide,
//           parent->innerBounds.maxWidth,
//           parent->lines[lineInParent].x
//    );
}

void Pesto::Element::newLine() {

    //necessary? So highest x == size.width
//    innerBounds.x += style.paddingRight.value();
//    innerBounds.x += style.borderRight.getWidth();

    lines.push_back(Pesto::ElementLine{
        innerBounds.x,
        innerBounds.y,
    });

//    printf("%s: Got newLine (%zu): %dx%d\n", name().c_str(), lines.size()-1, innerBounds.x, innerBounds.y);

    innerBounds.y = innerBounds.height;
    innerBounds.x = 0;
    innerBounds.x += style.paddingLeft.value();
    innerBounds.x += style.borderLeft.getWidth();
}

Pesto::Element *Pesto::Element::deepNextElement(Element *until) {
    if (firstChild) {
        //find first ElementNode in all children
        for (auto &child: children) {
            if (until && child == until) {
                return nullptr;
            }

            if (child->isElementNode()) {
                return dynamic_cast<Pesto::Element *>(child);
            }
        }
    }

    //no children anymore, so go next sibling, or upwards
    return deepNextElementSibling(until);
}

Pesto::Element *Pesto::Element::nextElementSibling() {
    if (next) {
        for (Node *nextNode = next; nextNode; nextNode = nextNode->next) {
            if (nextNode->isElementNode()) {
                return dynamic_cast<Pesto::Element *>(nextNode);
            }
        }
    }

    return nullptr;
}

Pesto::Element *Pesto::Element::deepNextElementSibling(Element *until) {
    if (this == until) {
        return nullptr;
    }

    if (next) {
        for (Node *nextNode = next; nextNode; nextNode = nextNode->next) {
            if (until && next == until) {
                return nullptr;
            }
            if (nextNode->isElementNode()) {
                return dynamic_cast<Pesto::Element *>(nextNode);
            }
        }
    }

    //no sibling, got to parent's next
    if (parent) {
        auto currentParent = parent;
        while (currentParent) {
            if (until && until == currentParent) {
                return nullptr;
            }

            if (currentParent->next) {
                if (until && currentParent->next == until) {
                    return nullptr;
                }
                if (currentParent->next->isElementNode()) {
                    return dynamic_cast<Pesto::Element *>(currentParent->next);
                }
            }

            currentParent = currentParent->parent;
        }
    }

    return nullptr;
}

void Pesto::Element::setPosition(float x, float y) {
    needRecalculation();
    needRedraw();
    style.left = x;
    style.top = y;
    style.position = "absolute";
}

std::string Pesto::Element::getTagName() {
    return tagName;
}

bool Pesto::Element::hasAttribute(const std::string &name) {
    return 1 == attributes.count(name);
}

std::string Pesto::Element::getAttribute(const std::string &name) {
    if (!attributes.count(name)) {
        return std::string("");
    }

    return attributes[name];
}

bool Pesto::Element::hasPseudoClass(const std::string &className) {
    return styleState.hasPseudoClass(className);
}

bool Pesto::Element::hasClass(const std::string &className) {
    return styleState.hasClass(className);
}

Pesto::CSSParser *Pesto::Element::getCSSParser() {
    return cssParser;
}

void Pesto::Element::setCSSParser(Pesto::CSSParser *parser) {
    this->cssParser = parser;
}

Pesto::Element *Pesto::Element::getOffsetParent() {

    if (parent) {
        auto parent = dynamic_cast<Element *>(this->parent);
        if (!parent) {
            return nullptr;
        }

        if (parent->style.position == "absolute") {
            return parent;
        }

        if (parent->style.position == "relative") {
            return parent;
        }

        return parent->getOffsetParent();
    }

    return nullptr;
}

Pesto::Element::Element() {

    style.linkWithState(&styleState);
    styleState.id = &id;
    styleState.tagName = &tagName;

//    style.changed = [this](std::string name) {
//        this->styleChanged(name);
//    };
}

void Pesto::Element::setClass(const std::string &classes) {
    auto split = Pesto::utils::split(classes, ' ');
    styleState.clearClasses();

    for (auto &item: split) {
        styleState.addClass(item);
    }
}

void Pesto::Element::setStyles(const std::string &inlineStyles) {
    for (auto &[name, value]: CSSParser::parseInlineStyle(inlineStyles)) {
        style.setProperty(styleState, name, value);
    }
}

bool Pesto::Element::isScrollBarVisibleY() {
    return style.overflow == "scroll" ||
           (style.overflow == "auto" && innerBounds.height > size.height);
}

bool Pesto::Element::isScrollBarVisibleX() {
    return style.overflow == "scroll" ||
           (style.overflow == "auto" && innerBounds.width > size.width);
}

void Pesto::Element::styleChanged(const std::string &name) {
    //todo, we need to check where this change is coming from. From the CSSStyleApplier?
    //from manual setting a value? If latter, we need to flag that this element bacame style-dirty
    //and need to propagate the values down to all children (of course only for inherited style attributes)

    //do we need recalculation?
    //todo, implement if
    needRecalculation();
    //needRecalculation triggers a redraw automatically when needed based on dimension changes

    //do we need redraw? Maybe we changed colors, so we need a redraw as well.
    //todo, implement if
    needRedraw();
}

Pesto::Element *Pesto::Element::setText(std::string text) {
    if (children.size() == 1) {
        TextElement *textElement = dynamic_cast<TextElement *>(firstChild);
        if (textElement) {
            //we got exactly one children, which is already a textNode, so just set its text
            textElement->setText(text);
            needRecalculation();
            needRedraw();
            return this;
        }
    }

    //remove upside down
    while (lastChild) {
        auto copy = lastChild;
        removeChild(lastChild);
        delete copy;
    };

    auto textElement = new TextElement();
    textElement->setText(text);
    textElement->setParent(this);

    return this;
}

bool Pesto::Element::isElementNode() {
    return true;
}

Pesto::Element *Pesto::Element::parentElement() {
    if (!parent) {
        return nullptr;
    }

    return dynamic_cast<Pesto::Element *>(parent);
}

Pesto::Element *Pesto::Element::firstChildElement() {
    if (firstChild) {
        //find first ElementNode in all children
        for (auto &child: children) {
            if (child->isElementNode()) {
                return dynamic_cast<Pesto::Element *>(child);
            }
        }
    }

    return nullptr;
}

void Pesto::Element::appendChild(Pesto::Element *child) {
    child->setParent(this);
}

Pesto::Element::Element(std::string tagName) : Element() {
    this->tagName = tagName;
}

Pesto::Element *Pesto::Element::createChildren(std::string tagName) {
    auto element = new Pesto::Element();
    element->setParent(this);
    element->tagName = tagName;
    return element;
}

void Pesto::Element::needRestyle() {
    //from this element, to _all_ children, to all direct parents.
    auto document = getDocument();
    if (document) {
        document->restyle();
    }
}

Pesto::Document *Pesto::Element::getDocument() {
    if (parent) {
        return parentElement()->getDocument();
    }

    return nullptr;
}

Pesto::Window *Pesto::Element::getWindow() {
    auto doc = getDocument();

    if (doc) {
        return doc->getWindow();
    }

    return nullptr;
}

void Pesto::Element::addClickListener(std::function<void(Pesto::MouseEvent)> callback) {
    auto win = getWindow();

    if (!win) {
        throw std::runtime_error(utils::format("No window assigned to element: %s\n", path().c_str()));
    }

    win->addClickListener(this, callback);
}
