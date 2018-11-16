//
// Created by marc on 17.07.18.
//

#ifndef DEEPKIT_MODULE_H
#define DEEPKIT_MODULE_H

#include <unordered_map>
#include <functional>
#include "Button.h"
#include "../Element.h"

namespace Pesto {

    class Module {
    public:
        std::unordered_map<std::string, std::function<RegistryController *(Element *)>> components;

        Module() {
            components["button"] = Button::factory;
        }
    };
}

#endif //DEEPKIT_MODULE_H
