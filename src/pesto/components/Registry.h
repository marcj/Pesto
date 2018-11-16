//
// Created by marc on 17.07.18.
//

#ifndef DEEPKIT_REGISTRY_H
#define DEEPKIT_REGISTRY_H

#include "../Element.h"
#include "./Module.h"
#include "RegistryController.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace Pesto {
    class Registry {
    public:
        typedef RegistryController *factory(Element *);

        static std::unordered_map<std::string, std::function<factory>> controllers;

        static void registerComponent(const std::string &name, std::function<factory> factory) {
            Registry::controllers[name] = factory;
        }

        static void registerModule(Module *module) {
            for (auto &[n, c]: module->components) {
                Registry::registerComponent(n, c);
            }
        }
    };
}


#endif //DEEPKIT_REGISTRY_H
