//
// Created by marc on 15.06.18.
//

#ifndef DEEPKIT_EVENT_H
#define DEEPKIT_EVENT_H

#include "SDL.h"

namespace Pesto {
    class Element;

    class Event {
    public:
        Element *target = nullptr;
    };

    class KeyboardEvent: public Event {
    public:
        std::string key = "";
        SDL_Keycode keyCode = 0;
        bool metaKey = false;
        bool shiftKey = false;
        bool ctrlKey = false;

        void reset() {
            key = "";
            keyCode = 0;
            metaKey = false;
            shiftKey = false;
            ctrlKey = false;
        }

        std::string toString() {
            std::string str;
            if (metaKey) str.append(" meta");
            if (shiftKey) str.append(" shift");
            if (ctrlKey) str.append(" ctrl");
            if (!key.empty()) str.append(" " + key);

            return str;
        }
    };

    class MouseEvent: public Event {
    public:
        int button = 0;
        int screenX = 0;
        int screenY = 0;
        int stopped = false;
        int wheelX = 0;
        int wheelY = 0;

        void stopPropagation() {
            stopped = true;
        }
    };
}

#endif //DEEPKIT_EVENT_H
