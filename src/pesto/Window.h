//
// Created by marc on 01.06.18.
//

#pragma once
#include <exception>
#include <string>

// necessary when Skia compiled without debug stuff, (is_official_build=true)
#define GR_GL_CHECK_ERROR 0
#define GR_GL_LOG_CALLS 0

#include "gl/GrGLConfig.h"

#include "SDL.h"
#include "Document.h"
#include "Base.h"
#include "Element.h"
#include "gl/GrGLInterface.h"
#include "Event.h"
#include "core/Timer.cpp"

namespace Pesto {

    class WindowRegistry;

    class sgl_error : public std::exception {
    public:
        const char *message;

        sgl_error(const char *message) : message(message) {

        }

        const char *what() const _NOEXCEPT {
            const char *error = SDL_GetError();
            SDL_ClearError();

            return std::string(message).append(": ").append(error).c_str();
        }
    };

    class Window: public LifeTimeEventDispatcher {

    private:
        Size size = Size::create(300, 300);
        XYPosition position;
        KeyboardEvent keyboardEvent;

    public:
        SDL_Window *window = nullptr;
        Window *parent = nullptr;

        SkCanvas *canvas = nullptr;
        SkCanvas *cacheCanvas = nullptr;
        sk_sp<SkImage> cacheImage;

        Document *document = nullptr;
        Document *debugger = nullptr;
//        Element *root = nullptr;
        WindowRegistry *windowRegistry = nullptr;

        Uint32 windowId = 0;

        MouseEvent listenerEvent;
        sk_sp<SkSurface> surface;
        sk_sp<SkSurface> cacheSurface;

        //will be deleted when surface is destroyed
        SkCanvas *hardwareCanvas = nullptr;
        sk_sp<const GrGLInterface> interface;
        SDL_GLContext glContext;
        sk_sp<GrContext> grContext;
        Element *lastHoveredElement = nullptr;

        EventDispatcher<bool> onclose;
        EventDispatcher<KeyboardEvent> onkeydown;
        EventDispatcher<KeyboardEvent> onkeypress;
        EventDispatcher<KeyboardEvent> onkeyup;
        EventDispatcher<Event> onmove;
        EventDispatcher<MouseEvent> onclick;
        EventDispatcher<MouseEvent> onwheel;
        EventDispatcher<MouseEvent> onmousemove;

//        Window *debugger = nullptr;

        Window(WindowRegistry *windowRegistry);

        Element *fromXML(const std::string &xml);

        virtual ~Window();

        Timer tickTimer;

        void handleEvent(SDL_Event *event);

        Element *findElement(Element * root, float x, float y);

        void openDebugger();

        void setParent(Window *parent);
        void setPosition(float width, float height);

        void setSize(float width, float height);

        void setSize(Size size);

        void addClickListener(Element *element, std::function<void (MouseEvent)> callback);

        SkCanvas *createCanvas(int width, int height);

        void tick();
    };

}
