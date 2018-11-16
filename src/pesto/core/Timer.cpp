//
// Created by marc on 07.06.18.
//

#ifndef DEEPKIT_TIMER_H
#define DEEPKIT_TIMER_H

#include <iostream>
#include <chrono>

namespace Pesto {
    class Timer {
    public:
        Timer() : beg_(clock_::now()) {}

        void reset() { beg_ = clock_::now(); }

        double elapsed() const {
            return std::chrono::duration_cast<second_>(clock_::now() - beg_).count();
        }

        void printElapsed(const std::string &title) {
            printf("%s took: %fms\n", title.c_str(), this->elapsed() * 1000);
        }

    private:
        typedef std::chrono::high_resolution_clock clock_;
        typedef std::chrono::duration<double, std::ratio<1> > second_;
        std::chrono::time_point<clock_> beg_;
    };
}

#endif