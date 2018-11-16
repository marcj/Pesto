//
// Created by marc on 04.06.18.
//

#include "utils.h"

#include <iomanip>
#include <sstream>
#include <ios>
#include <signal.h>

#if defined(__linux__) && !defined(__GLIBC__) || \
    defined(__UCLIBC__) || \
    defined(_AIX)
#define HAVE_EXECINFO_H 0
#else
#define HAVE_EXECINFO_H 0
#endif

#if HAVE_EXECINFO_H
#include <execinfo.h>
#include <libunwind.h>
#include <dlfcn.h>    // for dladdr
#include <cxxabi.h>   // for __cxa_demangle
#include <cstdio>
#endif

std::string Pesto::utils::toHex(int i) {
    std::stringstream stream;
    stream << "0x"
           << std::setfill('0') << std::setw(sizeof(int) * 2)
           << std::hex << i;

    return stream.str();
}

std::string Pesto::utils::toStringHex(int i) {
    std::stringstream stream;
    stream << "#"
           << std::setfill('0') << std::setw(sizeof(int) * 2)
           << std::hex << i;

    return stream.str();
}

std::vector<std::string> Pesto::utils::split(const std::string &str, const char &splitter) {
    std::vector<std::string> strings;
    std::istringstream f(str);
    std::string s;
    while (getline(f, s, splitter)) {
        strings.push_back(s);
    }

    return strings;
}

std::string Pesto::utils::replace(const std::string &str, const char &oldChar, const char &newChar) {
    std::string copy = str;
    std::replace( copy.begin(), copy.end(), oldChar, newChar);

    return copy;
}

std::string Pesto::utils::removeChar(const std::string &str, const char &character) {
    std::string copy = str;
    copy.erase(std::remove(copy.begin(), copy.end(), character), copy.end());

    return copy;
}

void print_backtrace(int skip = 1) {

#if HAVE_EXECINFO_H
    void *callstack[128];
    const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
    int nFrames = backtrace(callstack, nMaxFrames);
    char **symbols = backtrace_symbols(callstack, nFrames);

    for (int i = skip; i < nFrames; i++) {
        Dl_info info;

        if (dladdr(callstack[i], &info) && info.dli_sname) {
            char *demangled = NULL;
            int status = -1;

            if (info.dli_sname[0] == '_') {
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            }

            fprintf(stderr, "%-3d %*p %s + %zd\n",
                    i - skip,
                    int(2 + sizeof(void *) * 2),
                    callstack[i],
                    status == 0 ? demangled : info.dli_sname == 0 ? symbols[i] : info.dli_sname,
                    (char *) callstack[i] - (char *) info.dli_saddr
            );
            free(demangled);
        } else {
            fprintf(stderr, "%-3d %*p %s\n",
                    i - skip,
                    int(2 + sizeof(void *) * 2),
                    callstack[i],
                    symbols[i]
            );
        }
    }
    free(symbols);
#endif  // HAVE_EXECINFO_H
}

void Pesto::utils::setupErrorListener() {
    std::set_terminate([] {
        std::exception_ptr p = std::current_exception();

        try {
            std::rethrow_exception(p);
        } catch (const std::exception &e) {
            printf("Uncaught exception %s:\n", e.what());
        }

        print_backtrace(5);
        exit(1);
    });

    signal(SIGSEGV, [](int sig) {
        print_backtrace(3);
        exit(1);
    });

    signal(SIGABRT, [](int sig) {
        print_backtrace(3);
        exit(1);
    });
}
