//
// Created by marc on 02.06.18.
//

#include "Application.h"
#include "WindowRegistry.h"
#include "components/Module.h"
#include "components/Registry.h"

void Pesto::Application::tick() {
    SDL_Event event;
    static int count = 0;

    while (SDL_PollEvent(&event) > 0) {
        count++;
//        printf("[%d] event %#08x\n", event.window.windowID, event.type);
        if (event.window.windowID) {
            if (windowRegistry.has(event.window.windowID)) {
                Window *window = windowRegistry.windows[event.window.windowID];

                if (window) {
                    window->handleEvent(&event);
                }

                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    delete window;
                }
            }
        }

        if (windowRegistry.windows.empty()) {
            running = false;
        }

        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            default:
                break;
        }
    }

    //render
    if (running) {
        for (auto const &[key, value]: windowRegistry.windows) {
            value->tick();
        }
    }
}

/**
 * This is necessary, since  SDL_PollEvent won't catch resize events as they happen but only the last 3
 * when resize finished. So windows black during resize without this code.
 */
int filterEvent(void *userdata, SDL_Event * event) {
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
        Pesto::Application *app = (Pesto::Application *)userdata;

        if (event->window.windowID) {
            if (app->windowRegistry.has(event->window.windowID)) {
                Pesto::Window *window = app->windowRegistry.windows[event->window.windowID];

                if (window) {
                    //DANGEROUS: THIS MIGHT BE CALLED FROM WITHING A THREAD FROM SDL2.
                    //WE NEED TO FIGURE OUT IF THIS IS MAKING TROUBLES OR NOT.
                    window->handleEvent(event);
                    window->tick();
                }

            }
        }

        return 0;
    }

    return 1;
}

Pesto::Application::Application() {

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    static const int kStencilBits = 8;  // Skia needs 8 stencil bits

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, kStencilBits);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        throw std::runtime_error("Could not init SDL");
    }

    SDL_SetEventFilter(filterEvent, this);

    Registry::registerModule(new Module);
}


Pesto::Application::~Application() {
    this->running = false;

    //Quit SDL subsystems
    SDL_Quit();
}

void Pesto::Application::loop() {
    uint32_t timeForOneFrame = (uint32_t) (1000 / this->fps);
    uint32_t took;
    uint32_t start = 0;
    uint32_t diff;

    while (this->running) {
        diff = SDL_GetTicks() - start;
        start = SDL_GetTicks();

        for (auto &item : animationCallbacks) {
            item(diff);
        }

        this->tick();

        took = (SDL_GetTicks() - start);

        if (took < timeForOneFrame) {
            //we have time left to draw the next frame.
            SDL_Delay(timeForOneFrame - took);
        }
    }
}

void Pesto::Application::addAnimationFrameCallback(std::function<void(float)> callback) {
    animationCallbacks.push_back(callback);
}

Pesto::Window *Pesto::Application::createWindow() {
    return windowRegistry.createWindow();
}

Pesto::Window *Pesto::Application::createWindow(const std::string &xml) {
    return windowRegistry.createWindow(xml);
}
