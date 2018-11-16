//
// Created by marc on 02.06.18.
//
#pragma once

#include "SDL.h"
#include "Window.h"
#include "WindowRegistry.h"

namespace Pesto {
    class Application {

    public:
        bool running = true;
        int fps = 60;
        uint32_t windowFlags;
        WindowRegistry windowRegistry;

        Application();
        virtual ~Application();

        Window *createWindow();

        Window *createWindow(const std::string &xml);

        std::vector<std::function<void(float)>> animationCallbacks;

        void addAnimationFrameCallback(std::function<void(float)> callback);

        void loop();

        void tick();
    };
}
