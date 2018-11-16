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
    width: 500px;
    display: inline;
}

.first {
    background-color: #ddd;
    padding: 10px;
    color: black;
    margin-bottom: 5px;
    border-bottom: 1px solid red;
}

.first div {
    background-color: white;
    border: 1px solid black;

    height: 50px;
    line-height: 100%;

    text-align: center;
    vertical-align: middle;
}

root.second .first div {
    border-color: red;
    background-color: #aaa;
    color: white;
}

.first .bunt {
    border-radius: 25px;
    border-top: 5px solid silver;
    border-right: 5px solid blue;
    border-bottom: 5px solid black;
    border-left: 5px solid red;
}

.first .bunt2 {
    border-radius: 5px;
    border-top: 5px solid red;
    border-right: 1px solid blue;
    border-left: 1px solid blue;
    border-bottom: 1px solid blue;
}
</style>
<root>
  <div class="first">
    <div style="">1px width / 1px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px">5px width / 0px radius</div>
  </div>
  <div class="first">
    <div style="border-radius: 1px;">1px width / 1px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-radius: 5px;">5px width / 5px radius</div>
  </div>
  <div class="first">
    <div style="border-radius: 5px;">1px width / 5px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-radius: 15px;">5px width / 15px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 15px;">5px 15px width / 15px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 25px;">5px 15px width / 25px radius</div>
  </div>
  <div class="first">
    <div class="bunt">Bunt</div>
  </div>
</root>
<root class="second">
  <div class="first">
    <div style="">1px width / 1px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px">5px width / 0px radius</div>
  </div>
  <div class="first">
    <div style="border-radius: 1px;">1px width / 1px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-radius: 5px;">5px width / 5px radius</div>
  </div>
  <div class="first">
    <div style="border-radius: 5px;">1px width / 5px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-radius: 15px;">5px width / 15px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 15px;">5px 15px width / 15px radius</div>
  </div>
  <div class="first">
    <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 25px;">5px 15px width / 25px radius</div>
  </div>
  <div class="first">
    <div class="bunt2">Bunt 2</div>
  </div>
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