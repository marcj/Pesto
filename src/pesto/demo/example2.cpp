#import "../Base.h"
#import "../Window.h"
#import "../Application.h"
#import "../Element.h"
#import "../TextElement.h"

int main(int argc, char **argv) {
    for (auto arg = argv; *arg; arg++) {
        printf("%s\n", *arg);
    }

    auto *application = new Pesto::Application;
    Pesto::utils::setupErrorListener();

    auto *window = application->createWindow();
    window->setSize(900, 400);

    std::vector<Pesto::Element *> elements;

    int labels = 100;

    int cols = 15;
    for (int i = 0; i < labels; i++) {
        Pesto::Element *element = new Pesto::Element();

        element->setStyles("background-color: #eee; color: blue;");
        element->setParent(window->document);
        element->setPosition((i % cols) * 60, (int) std::ceil(i / cols) * 20);
        element->setText(std::to_string(i));
        elements.push_back(element);
    }

    printf("Started\n");

    int counter = 0;
    bool activeChanges = true;

    window->onclick.listen([&] (Pesto::MouseEvent){
        activeChanges = !activeChanges;
    });

    application->addAnimationFrameCallback([&](float diff) {
        if (activeChanges) {
            counter++;
            for (int i = 0; i < labels; i++) {
                elements[i]->setText(
                    std::string().append(std::to_string(i)).append(": ").append(std::to_string(counter)));
            }
        }
    });

    application->fps = 100;
    application->loop();

    printf("Done\n");
    return 0;
}