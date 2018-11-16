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
    padding-top: 50px;
    padding-left: 50px;
    padding-right: 50px;
    padding-bottom: 50px;
    color: black;
    margin-bottom: 5px;
    border: 1px solid gray;
}

</style>
<root>
  <div class="first">
    <div style="height: 20px; background: red;"></div>
    Hi
  </div>
  <div class="first">
    <div style="height: 20px; background: orange;"></div>
    Hi2
  </div>
  <div class="first">
    <div style="height: 20px; background: green;"></div>
    Hi3
  </div>
</root>
)";

    auto *window = application->createWindow(xml);
    window->setSize(600, 400);

    std::cout << window->document->toString();

    printf("Started\n");

    application->loop();

    printf("Done\n");
    return 0;
}