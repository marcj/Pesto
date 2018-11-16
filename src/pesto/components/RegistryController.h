//
// Created by marc on 18.07.18.
//

#ifndef DEEPKIT_REGISTRYCONTROLLER_H
#define DEEPKIT_REGISTRYCONTROLLER_H

namespace Pesto {
    class Element;

    class RegistryController {
    public:
        Element *element;

        explicit RegistryController(Element *element) : element(element) {}

        virtual std::string getStyle() {
            return u8R"()";
        }
    };
}

#endif //DEEPKIT_REGISTRYCONTROLLER_H
