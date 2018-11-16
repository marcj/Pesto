# Pesto GUI


WORK IN PROGRESS.

Pesto is a high-performance GUI framework in C++ highly inspired by CSS and HTML,
using Skia as rendering engine.

It's based on

  - Clang (so ready for WebAssembly)
  - C++17
  - Cmake
  - SDL2


## Example

![Example screenshot](https://raw.github.com/marcj/Pesto/master/docs/assets/screenot-example_absolute.png)

```c++
#import "pesto/Base.h"
#import "pesto/Window.h"
#import "pesto/Application.h"
#import "pesto/Element.h"

#include <iostream>
#include <string>

int main(int argc, char **argv) {
    for (auto arg = argv; *arg; arg++) {
        printf("%s\n", *arg);
    }

    auto *application = new Pesto::Application;
    Pesto::utils::setupErrorListener();

    // language=HTML
    std::string xml = u8R"(
<style>
.left {
    position: absolute;
    top: 5px;
    left: 5px;
    width: 245px;
    bottom: 5px;
    padding: 5px;
    background-color: #eee;
}

sidebar-splitter {
    position: absolute;
    right: 0px;
    top: 0;
    width: 3px;
    bottom: 0;
    background-color: black;
}

content {
    position: absolute;
    left: 250px;
    padding: 5px;
    top: 5px;
    right: 55px;
    bottom: 5px;
    background-color: #bbb;
}

.right {
    position: absolute;
    width: 50px;
    background: red;
    top: 5px;
    right: 5px;
    bottom: 5px;
}

#footer {
    position: absolute;
    left: 0;
    right: 0;
    bottom: 0px;
    height: 50px;
    background: gray;
    color: white;
}
</style>
<div class="left">
    Hello Sidebar
    <sidebar-splitter>
    </sidebar-splitter>
</div>
<content>
    <div id="content" cacheRender="true">
        Hello Content
    </div>

    <button>Button 1</button>
    <button>Button 2</button>
    <div>
        Hello!
    </div>

    <div id="footer">
        Hello Footer
    </div>
</content>
<div class="right">
</div>
)";

    auto *window = application->createWindow(xml);
    window->setSize(600, 400);

    std::cout << window->document->toString();

    printf("Started\n");

    auto content = window->document->queryOne("#content");
    unsigned int counter = 0;

    application->addAnimationFrameCallback([&](float diff) {
        content->setText(std::string("Hi #").append(std::to_string(counter++)));
    });

    application->loop();

    printf("Done\n");
    return 0;
}
```

### Supported CSS

```
    - position: absolute | relative | static
    - left, top, right, bottom in px and %
    - width, height in px and %
    - overflow: auto | scroll | hidden | visible
    - text-align: left | center | right
    - vertical-align: left | middle | right
    - display: block | inline
    - border, border-left, border-right, border-top
    - border-radius, border-top-left-radius, etc
    - padding, padding-left, padding-right, padding-top, padding-right
    - margin, margin-left, margin-right, margin-top, margin-right
    - max-width, max-height
    - min-width, min-height
    - font-family
    - line-height
    - font-size
    - font-weight
    - color: hex, rgba, etc
    - white-space: pre | pre-wrap | pre-line | nowrap
    - background-color
```

## Install dependencies

### Build Skia
```
git submodule init
export PATH="${PWD}/libs/depot_tools:${PATH}"

cd libs/skia
python tools/git-sync-deps
bin/gn gen out/Clang --args='is_official_build=true cc="clang" cxx="clang++" skia_use_dng_sdk=false skia_use_sfntly=false skia_use_libwebp=false'

# bin/gn gen out/ClangO2 --args='is_official_build=true is_debug=false extra_cflags=["-O2"] cc="/usr/local/Cellar/llvm@5/5.0.1/bin/clang" cxx="/usr/local/Cellar/llvm@5/5.0.1/bin/clang++" skia_use_dng_sdk=false skia_use_sfntly=false skia_use_libwebp=false'

ninja -C out/ClangO
```


### OSX

Make sure to install on OSX `libjpeg-turbo`, `webp`, `icu4c`, `sdl2` via brew.

```
brew install libjpeg-turbo webp icu4c
```

On OSX it's important to make libjpeg-turbo headers available to standard include path (which overwrite default `libjpeg`),
so use `brew link --force --overwrite libjpeg-turbo`.

Same with icu4c, so call `brew link icu4c --force`.

## Use it

Use Cmake to build examples or pesto as library and include it in your project.
CLion works perfectly fine ;)


```
cmake --build ./src/pesto/cmake-build-debug --target pesto_demo_example_absolute -- -j 6

./src/pesto/cmake-build-debug/demo/pesto_demo_example_absolute
```


### WebAssembly

TBD

1. Apply patch
 - https://bugs.chromium.org/p/skia/issues/detail?id=8041#c12
 - https://github.com/Zubnix/skia-wasm-port/blob/master/wasm_compatible_build.patch