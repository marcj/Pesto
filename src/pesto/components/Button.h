//
// Created by marc on 15.07.18.
//

#ifndef DEEPKIT_BUTTON_H
#define DEEPKIT_BUTTON_H

#include <string>
#include <memory>

#include "RegistryController.h"
#include "../Element.h"
#include "../Window.h"

namespace Pesto {
    class Button: public RegistryController {
    public:
        Button(Element *element);

        std::string getStyle() override {
            return u8R"(
button {
    border: 1px solid gray;
    border-radius: 1px;
    background-color: #eee;
    padding-left: 5px;
    padding-right: 5px;
    display: inline;
    margin-left: 2px;
    margin-right: 2px;
}

button:hover {
    background-color: #ddd;
}
)";
        }

        static Button *factory(Element *element) {
            //mount link

            return new Button(element);
        }

        virtual void setTitle(std::string title) {

        }

        void onclick() {
            printf("Click\n");
        }
    };

    class ButtonNoop : public Button {
    public:
        ButtonNoop(Element *element) : Button(element) {}

        void setTitle(std::string title) override {

        }
    };
}

#endif //DEEPKIT_BUTTON_H
