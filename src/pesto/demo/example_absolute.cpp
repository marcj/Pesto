#import "../Base.h"
#import "../Window.h"
#import "../Application.h"
#import "../Element.h"

#include <iostream>
#include <string>

int main(int argc, char **argv) {
    for (auto arg = argv; *arg; arg++) {
        printf("%s\n", *arg);
    }

    auto *application = new Pesto::Application;
    Pesto::utils::setupErrorListener();

    // language=HTML
    std::string xml = u8R"(
<style>
.left {
    position: absolute;
    top: 5px;
    left: 5px;
    width: 245px;
    bottom: 5px;
    padding: 5px;
    background-color: #eee;
}

sidebar-splitter {
    position: absolute;
    right: 0px;
    top: 0;
    width: 3px;
    bottom: 0;
    background-color: black;
}

content {
    position: absolute;
    left: 250px;
    padding: 5px;
    top: 5px;
    right: 55px;
    bottom: 5px;
    background-color: #bbb;
}

.right {
    position: absolute;
    width: 50px;
    background: red;
    top: 5px;
    right: 5px;
    bottom: 5px;
}

#footer {
    position: absolute;
    left: 0;
    right: 0;
    bottom: 0px;
    height: 50px;
    background: gray;
    color: white;
}
</style>
<div class="left">
    Hello Sidebar
    <sidebar-splitter>
    </sidebar-splitter>
</div>
<content>
    <div id="content" cacheRender="true">
        Hello Content
    </div>

    <button>Button 1</button>
    <button if="showButton2 && active">Button 2</button>
    <div repeat="row in rows">
        Hello!
    </div>

    <div id="footer">
        Hello Footer
    </div>
</content>
<div class="right">
</div>
)";

    auto *window = application->createWindow(xml);
    window->setSize(600, 400);

    std::cout << window->document->toString();

    printf("Started\n");

    auto content = window->document->queryOne("#content");
    unsigned int counter = 0;

    application->addAnimationFrameCallback([&](float diff) {
        content->setText(std::string("Hi #").append(std::to_string(counter++)));
    });

    application->loop();

    printf("Done\n");
    return 0;
}