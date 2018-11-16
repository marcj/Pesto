//
// Created by marc on 08.06.18.
//

#ifndef DEEPKIT_CSSSTYLEAPPLIER_H
#define DEEPKIT_CSSSTYLEAPPLIER_H

#include "core/Timer.cpp"
#include "SelectorEngine.h"

namespace Pesto {
    class Element;

    class CSSStyleApplier {
    public:
        Pesto::Element &root;
        CSSStyleApplier(Pesto::Element &root);

        void applyStyles();
    };

}

#endif //DEEPKIT_CSSSTYLEAPPLIER_H
