//
// Created by marc on 12.06.18.
//

#ifndef DEEPKIT_WINDOWREGISTRY_H
#define DEEPKIT_WINDOWREGISTRY_H

#import "Window.h"

namespace Pesto {
    class WindowRegistry {
    public:
        std::unordered_map<uint32_t, Window *> windows;

        bool has(uint32_t windowId);

        void erase(uint32_t windowId);

        void registerWindow(Window *window);

        Window *createWindow(Window *parent);

        Window *createWindow(Window *parent, const std::string &xml);

        Window *createWindow();

        Window *createWindow(const std::string &xml);
    };
}

#endif //DEEPKIT_WINDOWREGISTRY_H
