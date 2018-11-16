//
// Created by marc on 04.06.18.
//

#ifndef DEEPKIT_TEXTELEMENT_H
#define DEEPKIT_TEXTELEMENT_H

#include "Node.h"
#include "SkTypeface.h"
#include "SkImage.h"

namespace Pesto {

    struct TextLine {
        std::string str;
        unsigned long line = 0;
        float x = 0;
        float y = 0;
    };

    class TextElement : public Node {
    sk_sp<SkTypeface> typeface;

    public:
        std::vector<TextLine> textLines;
        std::string text;
        sk_sp<SkImage> cached;
        double fps = 10;
        double drawTime = 0;

        void draw(SkCanvas *canvas, double diff) override;
        void calculateLayout(SkCanvas *canvas) override;

        void align() override;

        virtual void setText(std::string text);

        std::string name() override;

//        virtual void styleChanged(const std::string &name, const std::string &value) override;

        bool isTextNode() override;

//        void needRecalculation() override;
//        void needRedraw() override;
    };
}

#endif //DEEPKIT_TEXTELEMENT_H
