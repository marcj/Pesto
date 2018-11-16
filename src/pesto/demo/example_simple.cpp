#import "../Base.h"
#import "../Window.h"
#import "../Application.h"
#import "../Element.h"

#include <iostream>

int main(int argc, char **argv) {
    for (auto arg = argv; *arg; arg++) {
        printf("%s\n", *arg);
    }

    auto *application = new Pesto::Application;
    printf("1\n");
    Pesto::utils::setupErrorListener();
    printf("2\n");

    std::string xml = u8R"(
<style>
root {
    text-align: center;
}

span.box {
    display: inline;
    border: 1px solid blue;
    color: red;
}

#Hi1 {
    border: 5px solid black;
}

</style>
<root>
  <span class="box">Before</span>
    My first line My first line My first line
    My first line My first line My first line My first line My first line<br/>
    center
  <div id="Hi1" style="background: #eee; border-width: 5px; border-left-width: 15px; border-right-width: 15px;color: red;">Hi</div>

  Any yet another long line
</root>
)";
    printf("3\n");
    auto debuggerWindow1 = application->windowRegistry.createWindow(xml);
    printf("4\n");

    printf("Started\n");

//    debuggerWindow1->document->firstChildElement()->setText("Hello changed");

    auto first = debuggerWindow1->document->firstChildElement();
    printf("5\n");

    debuggerWindow1->onclick.listen([&](Pesto::MouseEvent e) {
        printf("Clicked: text-align=%s\n", first->style.textAlign.stringValue().c_str());

        int32_t c = SkColorSetARGB(255, (unsigned) rand() % 255, (unsigned) rand() % 255, (unsigned) rand() % 255);

        if (first->style.textAlign == "left") {
            first->style.textAlign = "center";
        } else if (first->style.textAlign == "center") {
            first->style.textAlign = "right";
        } else if (first->style.textAlign == "right") {
            first->style.textAlign = "left";
        }

        first->style.color = c;
        first->needRecalculation();
        first->needRedraw();
    });

    printf("6\n");
    std::cout << debuggerWindow1->document->toString();
    debuggerWindow1->openDebugger();
    printf("7\n");
    application->fps = 10;
    application->loop();
    printf("8\n");

    printf("Done\n");
    return 0;
}
