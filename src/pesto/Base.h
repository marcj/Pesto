//
// Created by marc on 01.06.18.
//

#ifndef DEEPKIT_BASE_H
#define DEEPKIT_BASE_H

#import "SkColor.h"
#include "core/utils.h"
#import <unordered_map>
#import <string>
#import <functional>
#import <vector>
#import <type_traits>

namespace Pesto {

    template<typename T>
    class EventDispatcherListener {
    public:
        int max = 0;
        int called = 0;
        const std::function<void(T)> *original = nullptr;
        std::function<void(T)> callback;

        explicit EventDispatcherListener(const std::function<void(T)> &callback, int max = 0) {
            original = &callback;
            this->callback = callback;
            this->max = max;
        }
    };

    template<typename T>
    class EventDispatcherBase {
    public:
        std::vector<EventDispatcherListener<T>> listeners;

        virtual ~EventDispatcherBase() {

        }

        virtual void offAll() {
            listeners.clear();
        }

//        typename std::enable_if<std::is_integral<T>::value, void>::type
//
//        template< typename =
//        typename std::enable_if< true >::type >

//        template <typename T>
//        template< typename =
//        typename std::enable_if<std::is_integral<T>::value>::type >

//        typename std::enable_if<std::is_same<T, bool>::value, void>
//        fire(){
//            for (auto &listener: listeners) {
//                listener->called++;
//                listener->callback(false);
//            }
//
//            Pesto::utils::remove(listeners, [](EventDispatcherListener<T> *listener) -> bool {
//                return listener->max > 0 && listener->called >= listener->max;
//            });
//        }

        virtual void fire(T arg) {
            for (auto &listener: listeners) {
                listener.called++;
                listener.callback(arg);
            }

            Pesto::utils::remove(listeners, [](EventDispatcherListener<T> listener) -> bool {
                return listener.max > 0 && listener.called >= listener.max;
            });
        }

        virtual void off(const std::function<void(T)> &callback) {
            Pesto::utils::remove(listeners, [&callback](EventDispatcherListener<T> listener) {
                return listener.original == &callback;
            });
        }

        virtual void listen(const std::function<void(T)> &callback) {
            listeners.push_back(EventDispatcherListener<T>(callback));
        }

        virtual void once(const std::function<void(T)> &callback) {
            listeners.push_back(EventDispatcherListener<T>(callback, 1));
        }

    };

    /**
     * This class should be used on elements that send their destruction to an event dispatcher,
     * that can be monitored to get an event when the object dies.
     */
    class LifeTimeEventDispatcher {
    public:
        EventDispatcherBase<bool> *destruction = nullptr;

        LifeTimeEventDispatcher() {
            destruction = new EventDispatcherBase<bool>;
        }

        virtual ~LifeTimeEventDispatcher() {
            destruction->fire(true);

            delete destruction;
        }
    };

    template<typename T>
    class EventDispatcher: public EventDispatcherBase<T> {
    public:
        using EventDispatcherBase<T>::listen;
        virtual void listenWhile(const std::function<void(T)> &callback, LifeTimeEventDispatcher &thisObjectIsAlive) {
            EventDispatcherBase<T>::listen(callback);
            thisObjectIsAlive.destruction->once([this, &callback](bool) {
                EventDispatcherBase<T>::off(callback);
            });
        }
    };

    struct XYPosition {
        float x = 0;
        float y = 0;

        XYPosition operator+(const XYPosition &position) const {
            return XYPosition{
                x + position.x,
                y + position.y,
            };
        }

        XYPosition operator+(const float wider) const {
            return XYPosition{
                x + wider,
                y + wider,
            };
        }
    };

    typedef SkAlpha Alpha;
    typedef SkColor Color;

    struct Size {
        float width = 0.0f;
        float height = 0.0f;

        bool widthInherit = true;
        bool heightInherit = true;

        static Size create(float width, float height) {
            return Size{width, height};
        }

        Size operator+(const Size &size) const {
            return Size{width + size.width, height + size.height};
        }
    };

    static constexpr inline Color SkColorSetARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
        return SkASSERT(a <= 255 && r <= 255 && g <= 255 && b <= 255),
            (a << 24) | (r << 16) | (g << 8) | (b << 0);
    }

    constexpr Color COLOR_TRANSPARENT = SkColorSetARGB(0x00, 0x00, 0x00, 0x00);
    constexpr Color COLOR_BLACK = SkColorSetARGB(0xff, 0x00, 0x00, 0x00);

    enum class BorderStyle {
        solid,
        dotted,
    };

    struct Border {
        float width = 0.0f;
        Color color = COLOR_TRANSPARENT;
        BorderStyle style = BorderStyle::solid;
    };
}

#endif //DEEPKIT_BASE_H
