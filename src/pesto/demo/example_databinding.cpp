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

    std::string xml = u8R"(
<root style="text-align: center;">
  My first line<br/>
  <div if="Hi" [bound]="true">
   Conditional display
  </div>
</root>
)";

    auto window = application->windowRegistry.createWindow(xml);

    std::cout << window->document->toString();

    application->fps = 10;
    application->loop();

    printf("Done\n");
    return 0;
}