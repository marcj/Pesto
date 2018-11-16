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

.first {
    background-color: #eee;
    height: 40px;
    padding: 5px;
    color: black;
    border-bottom: 1px solid gray;
}

.a {
    position: absolute;
    top: 50px;
    width: 250;
    background: #eee;
    padding-left: 25px;
}

.a strong {
    display: inline;
    background: white;
    padding: 5px;
    padding-top: 0px;
    border-size: 1px;
    border-color: red;
    margin: 5px;
}

.a .second {
    background: black;
    color: white;
    line-height: 26px;
    vertical-align: middle;
}

.b {
    position: absolute;
    left: 350;
    top: 30;
    width: 50;
    height: 30;
    background: gray;
    border: 1px solid red;
    borderColor: red;
}

#3 {
    position: absolute;
    left: 250;
    top: 250;r
    width: 50;
    height: 30;
    background: black;
    color: white;
}
</style>
<root>
  <div class="first">
    First Line
  </div>
  <div class="a">
    Hallo <strong>World</strong> Yes

    <div>New Line jooo</div>
    <div class="second">New Line middle</div>
    <div>New Line jooo</div>
    <div class="second">New Line middle</div>
    <div style="color: red;">New Line jooo</div>
    <div>New Line jooo</div>
    <div class="second" style="line-height: 40px; vertical-align: bottom;">New Line bottom</div>
    <div>New Line jooo</div>
    <div style="margin: 5px; background: #eee;">New Line jooo</div>
    <div style="margin: 5px; background: #aaa; line-height: 50px; margin-right: 15px; vertical-align: middle;">New Line middle</div>
    <div style="margin: 5px; background: #aaa; padding-top: 10px; margin-right: 15px;">
        <div style="width: 50px; margin: 5px; height: 10px; background: black;"></div>
        <div style="margin: 5px; height: 10px; background: black;"></div>
    </div>
    <div style="text-align: center">New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
    <div>New Line jooo</div>
  </div>
  <div class="b">Mo</div>
  <div id="3">Mowla</div>
</root>
)";

    application->fps = 10;
    auto *window = application->createWindow(xml);
    window->setSize(600, 600);

    std::cout << window->document->toString();

    printf("Started\n");

    application->loop();

    printf("Done\n");
    return 0;
}