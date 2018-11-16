//
// Created by marc on 12.06.18.
//

#ifndef DEEPKIT_DEBUGGERWINDOW_H
#define DEEPKIT_DEBUGGERWINDOW_H

#include <string>

namespace Pesto {
    std::string templateXml = u8R"(
<root>
    Hi!
</root>
)";

    class DebuggerComponent {
    public:
        static std::string templateXml;

    };
}


#endif //DEEPKIT_DEBUGGERWINDOW_H
