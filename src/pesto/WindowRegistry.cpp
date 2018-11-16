//
// Created by marc on 12.06.18.
//

#include "WindowRegistry.h"

bool Pesto::WindowRegistry::has(uint32_t windowId) {
    return WindowRegistry::windows.find(windowId) != WindowRegistry::windows.end();
}

Pesto::Window *Pesto::WindowRegistry::createWindow() {
    Window *window = new Window(this);

    return window;
}

Pesto::Window *Pesto::WindowRegistry::createWindow(const std::string &xml) {
    Window *window = createWindow();

    window->fromXML(xml);

    return window;
}

void Pesto::WindowRegistry::erase(uint32_t windowId) {
    printf("[%d] Window erased\n", windowId);
    windows.erase(windowId);
}

void Pesto::WindowRegistry::registerWindow(Pesto::Window *window) {
    windows[window->windowId] = window;
    printf("[%d] Window registered\n", window->windowId);
}

Pesto::Window *Pesto::WindowRegistry::createWindow(Pesto::Window *parent) {
    auto window = createWindow();
    window->setParent(parent);
    return window;
}

Pesto::Window *Pesto::WindowRegistry::createWindow(Pesto::Window *parent, const std::string &xml) {
    auto window = createWindow();
    window->setParent(parent);
    window->fromXML(xml);
    return window;
}