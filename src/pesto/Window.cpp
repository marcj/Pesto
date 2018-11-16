//
// Created by marc on 01.06.18.
//

#include "Window.h"
#include "Base.h"

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkSurface.h"
#include "SkGraphics.h"
#include "SkSurface_Gpu.h"
#include "GrRenderTargetContext.h"

#include "gl/GrGLUtil.h"
#include "DomReader.h"
#include "CSSStyleApplier.h"
#include "WindowRegistry.h"

#include <sstream>

#if defined(SK_BUILD_FOR_ANDROID)
#include <GLES/gl.h>
#elif defined(SK_BUILD_FOR_UNIX)
#include <GL/gl.h>
#elif defined(SK_BUILD_FOR_MAC)

#include <OpenGL/gl.h>
#include <SDL_video.h>

#elif defined(SK_BUILD_FOR_IOS)
#include <OpenGLES/ES2/gl.h>
#endif


Pesto::Element *Pesto::Window::findElement(Pesto::Element *root, float x, float y) {
    unsigned int foundInDepth = 0;
    Pesto::Element *target = nullptr;

    if (!root->hit(x, y)) {
        return nullptr;
    }

//    printf("FindElement root '%s' pos %.f/%.f, size=%.f/%.f  mouse=%.f/%.f => %d\n", root->name().c_str(),
//           root->globalPosition.x,
//           root->globalPosition.y,
//           root->size.width,
//           root->size.height,
//           x, y,
//           root->hit(x, y)
//    );

    for (Element *element = root; element;) {
//        printf("Check hit %s: %f/%f => %f/%f %fx%f -> %d [%d->%d]\n",
//               element->path().c_str(),
//               x - root->globalPosition.x, y - root->globalPosition.y,
//               element->globalPosition.x, element->globalPosition.y,
//               element->size.width, element->size.height, element->hit(x, y), foundInDepth, element->depth);

        if (element->hit(x, y) && foundInDepth <= element->depth) {
            target = element;
            foundInDepth = element->depth;

            //check if a deeper children is hit
            element = element->deepNextElement();
        } else {
            //don't try children, try siblings, or go to parent's sibling
//            element = element->deepNextElementSibling();
            element = element->deepNextElement();
        }

    }

    return target;
}


void Pesto::Window::handleEvent(SDL_Event *event) {

    if (event->type == SDL_WINDOWEVENT &&
        (event->window.event == SDL_WINDOWEVENT_RESIZED || event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)) {
        setSize(event->window.data1, event->window.data2);
    }

    if (event->type == SDL_WINDOWEVENT && (event->window.event == SDL_WINDOWEVENT_MOVED)) {
        position.x = (float) event->window.data1;
        position.y = (float) event->window.data2;

        Event listenerEvent;
        onmove.fire(listenerEvent);
    }

    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_LGUI:
                keyboardEvent.metaKey = true;
                break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                keyboardEvent.shiftKey = true;
                break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                keyboardEvent.ctrlKey = true;
                break;
            default:
                keyboardEvent.keyCode = event->key.keysym.sym;
                keyboardEvent.key = SDL_GetKeyName(event->key.keysym.sym);
        }

        if (event->key.repeat) {
            printf("[%d] Key PRESS '%s'\n", windowId, keyboardEvent.toString().c_str());
            onkeypress.fire(keyboardEvent);
        } else {
            printf("[%d] Key DOWN '%s'\n", windowId, keyboardEvent.toString().c_str());
            onkeydown.fire(keyboardEvent);
        }
    }

    if (event->type == SDL_KEYUP) {
        printf("[%d] Key UP '%s' (='%s')\n", windowId, SDL_GetKeyName(event->key.keysym.sym),
               keyboardEvent.toString().c_str());

        onkeyup.fire(keyboardEvent);

        switch (event->key.keysym.sym) {
            case SDLK_LGUI:
                keyboardEvent.metaKey = false;
                break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                keyboardEvent.shiftKey = false;
                break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                keyboardEvent.ctrlKey = false;
                break;
            default:
                keyboardEvent.keyCode = 0;
                keyboardEvent.key = "";
        }
    }

    if (event->type == SDL_MOUSEWHEEL) {
        listenerEvent.stopped = false;
        listenerEvent.wheelX = event->wheel.x * -5;
        listenerEvent.wheelY = event->wheel.y * -5;
        listenerEvent.target = findElement(document, listenerEvent.screenX, listenerEvent.screenY);

        if (!listenerEvent.target && debugger) {
            listenerEvent.target = findElement(debugger, listenerEvent.screenX, listenerEvent.screenY);
        }

        if (listenerEvent.target) {
            for (Element *i = listenerEvent.target; i; i = i->parentElement()) {
                if (listenerEvent.stopped) break;
                i->scrollX(listenerEvent);
                i->scrollY(listenerEvent);
            }
        }

        listenerEvent.stopped = false;
        onwheel.fire(listenerEvent);
    }

    if (event->type == SDL_MOUSEBUTTONUP) {
        listenerEvent.button = event->button.button;
        listenerEvent.screenX = event->button.x;
        listenerEvent.screenY = event->button.y;
        listenerEvent.wheelX = 0;
        listenerEvent.wheelY = 0;
        listenerEvent.target = findElement(document, listenerEvent.screenX, listenerEvent.screenY);

        if (!listenerEvent.target && debugger) {
            listenerEvent.target = findElement(debugger, listenerEvent.screenX, listenerEvent.screenY);
        }

        if (listenerEvent.target) {
            printf("SDL_MOUSEBUTTONUP: %s\n", listenerEvent.target->path().c_str());
        }

        onclick.fire(listenerEvent);
    }

    if (event->type == SDL_MOUSEMOTION) {
        listenerEvent.button = 0;
        listenerEvent.screenX = event->motion.x;
        listenerEvent.screenY = event->motion.y;

        listenerEvent.target = findElement(document, listenerEvent.screenX, listenerEvent.screenY);
        if (!listenerEvent.target && debugger) {
            listenerEvent.target = findElement(debugger, listenerEvent.screenX, listenerEvent.screenY);
        }

        if (lastHoveredElement != listenerEvent.target) {
            if (lastHoveredElement) {
                //element changed, so remove hover pseudoClass
                for (auto element = lastHoveredElement; element; element = element->parentElement()) {
                        element->styleState.removePseudoClass("hover");
                        element->needRecalculation();
                        element->needRedraw();
                }
            }

            //add hover to new element
            for (auto element = listenerEvent.target; element; element = element->parentElement()) {
                element->styleState.addPseudoClass("hover");
                element->needRecalculation();
                element->needRedraw();
            }

            if (listenerEvent.target) {
                printf("hovered element: %s\n", listenerEvent.target->path().c_str());
            }
        }

        onmousemove.fire(listenerEvent);
        lastHoveredElement = listenerEvent.target;
    }

//    printf("[%d] Handle event %#08x\n", windowId, event->type);
}


Pesto::Window::Window(WindowRegistry *windowRegistry) : windowRegistry(windowRegistry) {
    uint32_t windowFlags;

#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_IOS)
    // For Android/iOS we need to set up for OpenGL ES and we make the window hi res & full screen
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                  SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP |
                  SDL_WINDOW_ALLOW_HIGHDPI;
#else
    // For all other clients we use the core profile and operate in a window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
#endif

    // If you want multisampling, uncomment the below lines and set a sample count
//    static const int kMsaaSampleCount = 0; //4;
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, kMsaaSampleCount);

    // Setup window
//    // This code will create a window with the same resolution as the user's desktop.
//    if (SDL_GetDesktopDisplayMode(0, dm) != 0) {
//        throw sgl_error("Could not SDL_GetDesktopDisplayMode");
//    }

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
    window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, (int) size.width, (int) size.height, windowFlags);

    if (!window) {
        throw sgl_error("Could not SDL_CreateWindow");
    }

    // To go fullscreen
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    windowId = SDL_GetWindowID(window);

    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    position.x = (float) x;
    position.y = (float) y;

    // try and setup a GL context
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        throw sgl_error("Could not SDL_GL_CreateContext");
    }

    int success = SDL_GL_MakeCurrent(window, glContext);
    if (success != 0) {
        throw sgl_error("Could not SDL_GL_MakeCurrent");
    }

//    int dw, dh;
//    SDL_GL_GetDrawableSize(window, &dw, &dh);

    //SDL_GL_SetSwapInterval(1);

    glDisable(GL_DEPTH_TEST);
//    glViewport(0, 0, 500, 500);
//    glClearColor(1, 1, 1, 1);
//    glClearStencil(0);
//    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glViewport(0, 0, (int) size.width, (int) size.height);
    glClearColor(1, 1, 1, 1);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // setup GrContext
    interface = GrGLMakeNativeInterface();

    // setup contexts
    grContext = GrContext::MakeGL(interface);
    SkASSERT(grContext);

    delete canvas;
    canvas = createCanvas((int) size.width, (int) size.height);
    document = new Document(this);

    printf("[%d] Window up and running.\n", windowId);

    windowRegistry->registerWindow(this);

    onkeydown.listen([this](KeyboardEvent event) {
        if (event.shiftKey && event.ctrlKey && event.key == "I") {
            if (debugger) {
                document->size.height += debugger->size.height;
                destroy(debugger);
            } else {
                openDebugger();
            }
        }
    });
}

Pesto::Window::~Window() {
    SDL_GL_MakeCurrent(window, glContext);
    onclose.fire(true);

    destroy(debugger);

    windowRegistry->erase(windowId);

    //Destroy window
    destroy(document);

    SDL_GL_DeleteContext(glContext);

    //Destroy window
    SDL_DestroyWindow(window);
}

SkCanvas *Pesto::Window::createCanvas(int width, int height) {
//    uint32_t windowFormat = SDL_GetWindowPixelFormat(window);
    int contextType;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &contextType);

    // Wrap the frame buffer object attached to the screen in a Skia render target so Skia can
    // render to it
    GrGLint buffer;
    GR_GL_GetIntegerv(interface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    GrGLFramebufferInfo info;
    info.fFBOID = (GrGLuint) buffer;
    SkColorType colorType;

    uint32_t windowFormat = SDL_GetWindowPixelFormat(window);

    //SkDebugf("%s", SDL_GetPixelFormatName(windowFormat));
    // TODO: the windowFormat is never any of these?
    if (SDL_PIXELFORMAT_RGBA8888 == windowFormat) {
        info.fFormat = GR_GL_RGBA8;
        colorType = kRGBA_8888_SkColorType;
    } else {
        colorType = kBGRA_8888_SkColorType;
        if (SDL_GL_CONTEXT_PROFILE_ES == contextType) {
            info.fFormat = GR_GL_BGRA8;
        } else {
            // We assume the internal format is RGBA8 on desktop GL
            info.fFormat = GR_GL_RGBA8;
        }
    }

    int kStencilBits = 8;  // Skia needs 8 stencil bits
    int kMsaaSampleCount = 0; //4;
    GrBackendRenderTarget target(width, height, kMsaaSampleCount, kStencilBits, info);

    // setup SkSurface
    // To use distance field text, use commented out SkSurfaceProps instead
    // SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
    //                      SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    surface = SkSurface::MakeFromBackendRenderTarget(grContext.get(), target,
                                                     kBottomLeft_GrSurfaceOrigin,
                                                     colorType, nullptr, &props);

//    auto imageInfo = SkImageInfo::MakeN32Premul(width, height);
////    surface = SkSurface::SkSurface::MakeRaster(imageInfo);
//    surface = SkSurface::SkSurface::MakeR(grContext.get(), SkBudgeted::kNo, imageInfo);

    hardwareCanvas = surface->getCanvas();
    if (!hardwareCanvas) {
        throw std::runtime_error("Could not create canvas");
    }

    hardwareCanvas->clear(0xffffffff);
    hardwareCanvas->flush();

//    cacheSurface = SkSurface::MakeRenderTarget(hardwareCanvas->getGrContext(), SkBudgeted::kNo,
//                                               hardwareCanvas->imageInfo());
//    cacheCanvas = cacheSurface->getCanvas();

    return hardwareCanvas;
}

void Pesto::Window::tick() {
    int success = SDL_GL_MakeCurrent(window, glContext);
    if (success != 0) {
        throw sgl_error("Could not SDL_GL_MakeCurrent");
    }

//    printf("[%d] tick\n", windowId);

//    int success = SDL_GL_MakeCurrent(window, SDL_GL_GetCurrentContext());
//    if (success != 0) {
//        throw sgl_error("Could not SDL_GL_MakeCurrent");
//    }

    auto diff = tickTimer.elapsed();
    tickTimer.reset();

    canvas->clear(SK_ColorWHITE);

    document->tick(canvas, diff);

    if (debugger) {
        debugger->tick(canvas, diff);
    }

    canvas->flush();

    SDL_GL_SwapWindow(window);
}

void Pesto::Window::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
    SDL_SetWindowPosition(window, (int) position.x, (int) position.y);
}

void Pesto::Window::setSize(float width, float height) {
    setSize(Size::create(width, height));
}

void Pesto::Window::setSize(Size size) {
    if (this->size.width == size.width && this->size.height == size.height) {
        return;
    }

    SDL_GL_MakeCurrent(window, glContext);
    SDL_SetWindowSize(window, (int) size.width, (int) size.height);
    glViewport(0, 0, (int) size.width, (int) size.height);
    this->size = size;

    //todo, don't recreate it every time
    canvas = createCanvas((int) size.width, (int) size.height);

    this->document->size.width = size.width;
    this->document->size.height = size.height;

    if (debugger) {
        this->document->size.height -= debugger->size.height;

        this->debugger->size.width = size.width;
        debugger->globalPosition.y = size.height - debugger->size.height;

        this->debugger->needRecalculation();
        this->debugger->needRedraw();
    }

    this->document->needRecalculation();
    this->document->needRedraw();
}

Pesto::Element *Pesto::Window::fromXML(const std::string &xml) {
    return document->fromXML(xml);
}

void Pesto::Window::openDebugger() {
    debugger = new Document(this);

    std::string xml = u8R"(
<style>
pesto-debugger {
    background: #fefefe;
    position: relative;
    border-top: 3px solid gray;
    background-color: white;
    height: 100%;
}

.left {
    position: absolute;
    left: 0;
    top: 0;
    bottom: 0;
    right: 260px;
    overflow: auto;
    background-color: #fbfbfb;
}

.right {
    position: absolute;
    top: 0;
    right: 0;
    bottom: 0;
    width: 259px;
    padding-top: 5px;
    overflow: auto;
    border-left: 1px solid silver;
}

span {
    display: inline;
}

.margin, .border, .padding, .box {
    display: inline;
    padding: 5px;
    vertical-align: middle;
    border: 1px solid gray;
    background-color: #fafafa;
    margin-left: 5px;
    margin-right: 5px;
    font-size: 10px;
    position: relative;
    padding-top: 18px;
    padding-bottom: 18px;
}

.margin {
    line-height: 106px;
    background-color: #f9cc9d;
}

.border {
    line-height: 68px;
    background-color: #fddd9b;
}

.padding {
    line-height: 30px;
    background-color: #c3d08b;
}

.label {
    position: absolute;
    line-height: 14px;
    top: 0px;
    left: 15px;
    width: 100px;
}

.box {
    text-align: center;
    padding-top: 5px;
    padding-bottom: 5px;
    line-height: 100%;
    height: 30px;
    width: 140px;
    background-color: #8cb6c0;
}

.top {
    position: absolute;
    line-height: 14px;
    top: 0px;
    left: 0;
    right: 0;
    text-align: center;
}

.bottom {
    position: absolute;
    line-height: 24px;
    bottom: 0px;
    left: 0;
    right: 0;
    text-align: center;
}

#selected-text {
    border-bottom: 1px solid silver;
    padding: 5px;
}

#tree {
    padding: 5px;
}

#info {
    padding: 10px;
    white-space: pre-line;
}

#dimensionText {
    padding-top: 5px;
    text-align: center;
}

rule {
    border-top: 1px solid silver;
    padding: 2px;
}

rule-property {
    padding-left: 23px;
}

rule-property-name {
    color: #c80031;
    display: inline;
}

rule-property-value {
    display: inline;
}

</style>
<pesto-debugger>
<div class="left">
    <div id="selected-text">Nothing selected</div>
    <div id="tree"></div>
</div>
<div class="right">
    <div class="margin">
        <div class="label">margin</div>
        <div id="marginTop" class="top">0</div>
        <span id="marginLeft">0</span>
        <div class="border">
            <div class="label">border</div>
            <div id="borderTop" class="top">0</div>
            <span id="borderLeft">0</span>
            <div class="padding">
                <div class="label">padding</div>
                <div id="paddingTop" class="top">0</div>
                <span id="paddingLeft">0</span>
                <div class="box">0x0</div>
                <span id="paddingRight">0</span>
                <div id="paddingBottom" class="bottom">0</div>
            </div>
            <span id="borderRight">0</span>
            <div id="borderBottom" class="bottom">0</div>
        </div>
        <span id="marginRight">0</span>
        <div id="marginBottom" class="bottom">0</div>
    </div>
    <div id="dimensionText"></div>
    <div id="info"></div>
    <rules></rules>
</div>
</pesto-debugger>
)";
    debugger->fromXML(xml);

    auto selectedText = debugger->queryOne("#selected-text");
    auto boxText = debugger->queryOne(".box");
    auto tree = debugger->queryOne("#tree");
    auto dimensionText = debugger->queryOne("#dimensionText");
    auto infoText = debugger->queryOne("#info");
    auto paddingLeft = debugger->queryOne("#paddingLeft");
    auto paddingRight = debugger->queryOne("#paddingRight");
    auto paddingTop = debugger->queryOne("#paddingTop");
    auto paddingBottom = debugger->queryOne("#paddingBottom");

    auto marginLeft = debugger->queryOne("#marginLeft");
    auto marginRight = debugger->queryOne("#marginRight");
    auto marginTop = debugger->queryOne("#marginTop");
    auto marginBottom = debugger->queryOne("#marginBottom");

    auto borderLeft = debugger->queryOne("#borderLeft");
    auto borderRight = debugger->queryOne("#borderRight");
    auto borderTop = debugger->queryOne("#borderTop");
    auto borderBottom = debugger->queryOne("#borderBottom");
    auto rulesContainer = debugger->queryOne("rules");

    if (!selectedText) {
        throw std::runtime_error("Couldnt find that shit.");
    }

    //todo, reset this during onresize
    debugger->attributes["id"] = "debugger";
    debugger->size.width = size.width;
    debugger->size.height = 450;
    debugger->globalPosition.y = size.height - debugger->size.height;
    document->size.height -= debugger->size.height;

    std::unordered_map<Element *, Element *> *treeMap = new std::unordered_map<Element *, Element *>;

    for (Element *i = document; i; i = i->deepNextElement()) {
        auto div = debugger->createElement("div");
        div->setText("<" + i->name(true) + ">");
        div->style.paddingLeft = (int) i->depth * 10;
        tree->appendChild(div);
        (*treeMap)[i] = div;
    }

    debugger->destruction->listen([=](bool) {
        delete treeMap;
    });

    std::cout << debugger->toString() << "\n";

    onclick.listenWhile([=](MouseEvent e) {
        static Element *selected = nullptr;

        if (!e.target) {
            return;
        }
        if (selected && (*treeMap)[selected]) {
            (*treeMap)[selected]->style.color = "black";
            (*treeMap)[selected]->style.backgroundColor.unset();
        }

        selected = e.target;

        if ((*treeMap)[selected]) {
            (*treeMap)[selected]->style.backgroundColor = "#2196f3";
            (*treeMap)[selected]->style.color = "white";
        }

        auto text = utils::format("Clicked %dx%d on %s", e.screenX, e.screenY,
                                  e.target ? e.target->path().c_str() : "");

        selectedText->setText(text);
        std::stringstream info;

        info << "globalPosition.x: " << e.target->globalPosition.x << "\n";
        info << "globalPosition.y: " << e.target->globalPosition.y << "\n";
        info << "scrollTop: " << e.target->innerBounds.scrollTop << "\n";
        info << "scrollLeft: " << e.target->innerBounds.scrollLeft << "\n";
//        info << "innerBounds.x: " << e.target->innerBounds.x << "\n";
//        info << "innerBounds.y: " << e.target->innerBounds.y << "\n";
//        info << "innerBounds.maxWidth: " << e.target->innerBounds.maxWidth << "\n";
//        info << "innerBounds.maxHeight: " << e.target->innerBounds.maxHeight << "\n";
//        info << "lines: " << e.target->lines.size() << "\n";
//
//        for (auto &line: e.target->lines) {
//            info << utils::format("- line %.2fx%.2f", line.x, line.y) << "\n";
//        }

//        int i = 0;
//        for (auto child = e.target->firstChild; child; child = child->next) {
//            auto childText = dynamic_cast<TextElement *>(child);
//            if (!childText) continue;
//            i++;
//            info << "Text #" << i << "\n";
//            info << "- lines: " << childText->textLines.size() << "\n";
//            for (auto &line: childText->textLines) {
//                info << "- line[" << line.line << "].x=" << line.x << "\n";
//                info << "- line[" << line.line << "].y=" << line.y << "\n";
//            }
//        }

        unsigned long ruleCount = 0;
        if (e.target->styleState.hasInlineRule()) {
            ruleCount++;
        }
        for (auto &rule: e.target->styleState.rules) {
            if (rule->active(&e.target->styleState)) {
                ruleCount++;
            }
        }

        info << "CSS rules (" << ruleCount << "):\n";

        rulesContainer->destroyChildren();

        if (e.target->styleState.hasInlineRule()) {
            auto rule = e.target->styleState.getInlineRule();

            auto ruleElement = rulesContainer->createChildren("rule");
            ruleElement->createChildren("div")->setText("element.style {");

            for (auto &[name, value]: rule->properties) {
                auto propertyElement = ruleElement->createChildren("rule-property");
                propertyElement->createChildren("rule-property-name")->setText(name);
                propertyElement->createChildren("rule-property-value")->setText(": " + value + ";");
            }
            ruleElement->createChildren("div")->setText("}");
        }

        for (auto &rule: e.target->styleState.rules) {
            if (rule->active(&e.target->styleState)) {
                auto ruleElement = rulesContainer->createChildren("rule");
                ruleElement->createChildren("div")->setText(rule->selector.toString() + " {");

                for (auto &[name, value]: rule->properties) {
                    auto propertyElement = ruleElement->createChildren("rule-property");
                    propertyElement->createChildren("rule-property-name")->setText(name);
                    propertyElement->createChildren("rule-property-value")->setText(": " + value + ";");
                }
                ruleElement->createChildren("div")->setText("}");
            }
        }

        paddingLeft->setText(utils::format("%.0f", e.target->style.paddingLeft.value()));
        paddingRight->setText(utils::format("%.0f", e.target->style.paddingRight.value()));
        paddingTop->setText(utils::format("%.0f", e.target->style.paddingTop.value()));
        paddingBottom->setText(utils::format("%.0f", e.target->style.paddingBottom.value()));

        marginLeft->setText(utils::format("%.0f", e.target->style.marginLeft.value()));
        marginRight->setText(utils::format("%.0f", e.target->style.marginRight.value()));
        marginTop->setText(utils::format("%.0f", e.target->style.marginTop.value()));
        marginBottom->setText(utils::format("%.0f", e.target->style.marginBottom.value()));

        borderLeft->setText(utils::format("%d", e.target->style.borderLeft.getWidth()));
        borderRight->setText(utils::format("%d", e.target->style.borderRight.getWidth()));
        borderTop->setText(utils::format("%d", e.target->style.borderTop.getWidth()));
        borderBottom->setText(utils::format("%d", e.target->style.borderBottom.getWidth()));

        boxText->setText(utils::format(
            "%.2f x %.2f",
            e.target->size.width
            - (float) e.target->style.borderLeft.getWidth() - (float) e.target->style.borderRight.getWidth()
            - e.target->style.paddingLeft.value() - e.target->style.paddingRight.value(),

            e.target->size.height
            - (float) e.target->style.borderTop.getWidth() - (float) e.target->style.borderBottom.getWidth()
            - e.target->style.paddingTop.value() - e.target->style.paddingBottom.value()
        ));
        dimensionText->setText(utils::format(
            "%.2f x %.2f",
            e.target->size.width,
            e.target->size.height
        ));
        infoText->setText(info.str());

        this->debugger->needRecalculation();
        this->debugger->needRedraw();
    }, *debugger);
}

void Pesto::Window::setParent(Pesto::Window *parent) {
    this->parent = parent;
}

void Pesto::Window::addClickListener(Pesto::Element *element, std::function<void(MouseEvent)> callback) {
    onclick.listen([element, callback](MouseEvent e) {
       if (e.target == element) {
           callback(e);
       }
    });
}
