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

.third .first div {
    border-radius: 15px;
}

.fourth .first div {
    border: 0px solid black;
}
</style>
<root>
  <div class="first">
    <div style="border-top: 15px solid red;">top 15px;</div>
  </div>
  <div class="first">
    <div style="border-right: 15px solid red;">right 15px;</div>
  </div>
  <div class="first">
    <div style="border-bottom: 15px solid red;">bottom 15px;</div>
  </div>
  <div class="first">
    <div style="border-left: 15px solid red;">left 15px;</div>
  </div>
</root>
<root class="fourth">
  <div class="first">
    <div style="border-top: 1px solid red;">top 15px;</div>
  </div>
  <div class="first">
    <div style="border-right: 1px solid red;">right 15px;</div>
  </div>
  <div class="first">
    <div style="border-bottom: 1px solid red;">bottom 15px;</div>
  </div>
  <div class="first">
    <div style="border-left: 1px solid red;">left 15px;</div>
  </div>
</root>
<br/>
<root class="second">
  <div class="first">
    <div style="border-top: 15px solid red; border-right: 30px solid blue;">top 15px; right 30px</div>
  </div>
  <div class="first">
    <div style="border-right: 15px solid red; border-bottom: 30px solid blue;">right 15px; bottom 30px;</div>
  </div>
  <div class="first">
    <div style="border-bottom: 15px solid red; border-left: 30px solid blue;">bottom 15px; left 30px;</div>
  </div>
  <div class="first">
    <div style="border-left: 15px solid red; border-top: 30px solid blue;">left 15px; top 30px;</div>
  </div>
</root>
<root class="third">
  <div class="first">
    <div style="border-top: 15px solid red; border-right: 30px solid blue;">top 15px; right 30px</div>
  </div>
  <div class="first">
    <div style="border-right: 15px solid red; border-bottom: 30px solid blue;">right 15px; bottom 30px;</div>
  </div>
  <div class="first">
    <div style="border-bottom: 15px solid red; border-left: 30px solid blue;">bottom 15px; left 30px;</div>
  </div>
  <div class="first">
    <div style="border-left: 15px solid red; border-top: 30px solid blue;">left 15px; top 30px;</div>
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