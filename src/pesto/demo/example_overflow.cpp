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
    Pesto::utils::setupErrorListener();

    // language=HTML
    std::string xml = u8R"(
<style>
root {
    background-color: #ddd;
}

box {
    width: 250px;
    height: 150px;
    display: inline;
    background-color: white;
    border: 1px solid gray;
    margin: 10px;
    padding: 15px;
}

</style>
<root>
    <box id="1" style="overflow: scroll">
        <div style="border: 1px solid silver;">Block</div>
        This is a lovely box 1<br/>
        This is a lovely box 2<br/>
        This is a lovely box 3<br/>
        This is a lovely box 4<br/>
        This is a lovely box 5<br/>
        This is a lovely box 6<br/>
        This is a lovely box 7<br/>
        This is a lovely box 8<br/>
        This is a lovely box 9<br/>
        This is a lovely box 10<br/>
        This is a lovely box 11<br/>
    </box>
    <box id="2" style="overflow: auto">
        This is a lovely box 1<br/>
        This is a lovely box 2<br/>
        This is a lovely box 3<br/>
        This is a lovely box 4<br/>
        This is a lovely box 5<br/>
        This is a lovely box 6<br/>
        This is a lovely box 7<br/>
        This is a lovely box 8<br/>
        This is a lovely box 9<br/>
        This is a lovely box 10<br/>
        This is a lovely box 11<br/>
    </box>
    <box id="3" style="overflow: hidden">
        This is a lovely box 1<br/>
        This is a lovely box 2<br/>
        This is a lovely box 3<br/>
        This is a lovely box 4<br/>
        This is a lovely box 5<br/>
        This is a lovely box 6<br/>
        This is a lovely box 7<br/>
        This is a lovely box 8<br/>
        This is a lovely box 9<br/>
        This is a lovely box 10<br/>
        This is a lovely box 11<br/>
    </box>
    <box id="4">
        This is a lovely box 1<br/>
        This is a lovely box 2<br/>
        This is a lovely box 3<br/>
        This is a lovely box 4<br/>
        This is a lovely box 5<br/>
        This is a lovely box 6<br/>
        This is a lovely box 7<br/>
        This is a lovely box 8<br/>
        This is a lovely box 9<br/>
        This is a lovely box 10<br/>
        This is a lovely box 11<br/>
    </box>
</root>
)";

    auto *window = application->createWindow(xml);
    window->setSize(1200, 700);

    std::cout << window->document->toString();

    printf("Started\n");

    application->loop();

    printf("Done\n");
    return 0;
}