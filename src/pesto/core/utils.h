//
// Created by marc on 04.06.18.
//

#ifndef DEEPKIT_UTILS_H
#define DEEPKIT_UTILS_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cstdarg>

//delete the pointer and set it back to nullptr
template<class T> inline void destroy(T*& p) { delete p; p = nullptr; }

namespace Pesto {
    class utils {

    public:
        static void setupErrorListener();
        static std::string toStringHex(int i);
        static std::string toHex(int i);
        static std::vector<std::string> split(const std::string &str, const char &splitter);
        static std::string replace(const std::string &str, const char &oldChar, const char &newChar);
        static std::string removeChar(const std::string &str, const char &character);

        /**
         * Filters vector by filter in-place.
         */
        template<typename T, typename Func>
        static void remove(std::vector<T> &vector, const Func &filter){
            auto new_end = std::remove_if(vector.begin(), vector.end(), filter);
            vector.erase(new_end, vector.end());
        };

        template<typename T>
        static void remove(std::vector<T> &vector, T item){
            vector.erase(std::remove(vector.begin(), vector.end(), item), vector.end());
        };

        template<typename T>
        static bool has(std::vector<T> &vector, T item){
            return std::find(vector.begin(), vector.end(), item) != vector.end();
        };

        template<typename T, typename U>
        static bool has(std::unordered_map<T, U> &map, T key){
            return map.find(key) != map.end();
        };

        static std::string format(const std::string fmt, ...) {
            int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
            std::string str;
            va_list ap;
            while (1) {     // Maximum two passes on a POSIX system...
                str.resize(size);
                va_start(ap, fmt);
                int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
                va_end(ap);
                if (n > -1 && n < size) {  // Everything worked
                    str.resize(n);
                    return str;
                }
                if (n > -1)  // Needed size returned
                    size = n + 1;   // For null char
                else
                    size *= 2;      // Guess at a larger size (OS specific)
            }
            return str;
        }
    };
}

#endif //DEEPKIT_UTILS_H
