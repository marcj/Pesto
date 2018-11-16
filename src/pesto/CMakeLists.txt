cmake_minimum_required(VERSION 3.10)
project(pesto)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

ADD_DEFINITIONS(
    -std=c++1z
)

add_subdirectory(xml)
add_subdirectory(tests)
add_subdirectory(demo)

find_package(OpenGL REQUIRED)


add_library(pesto
    Window.cpp
    Document.cpp
    Application.cpp
    Element.cpp
    core/utils.cpp
    TextElement.cpp
    DomReader.cpp
    SelectorEngine.cpp
    CSSParser.cpp
    core/Timer.cpp
    CSSStyleApplier.cpp
    WindowRegistry.cpp
    Base.h
    DebuggerComponent.cpp
    Node.cpp
    Event.h
    TextCursor.cpp
    TextCursor.h
    Style2.h
    Style2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/tracy/TracyClient.cpp
    components/Button.cpp components/Button.h components/Registry.cpp components/Registry.h components/Module.h components/RegistryController.h)

target_include_directories(pesto
    PUBLIC ../../libs/skia/include/core
    PUBLIC ../../libs/skia/include/config
    PUBLIC ../../libs/skia/include/gpu
    PUBLIC ../../libs/skia/include/utils
    PUBLIC ../../libs/skia/third_party/externals/sdl/include/
    PUBLIC ../../libs/skia/src/gpu
    PUBLIC ../../libs/skia/src/core
    PUBLIC ../../libs/skia/src/image/
    PUBLIC ../../libs/skia/tools/
    PUBLIC ../../libs/skia/include/views/
    PUBLIC ../../libs/skia/include/effects/
    PUBLIC ../../libs/skia/include/private/
    PUBLIC ../../libs/skia/tools/timer/
    PUBLIC ../../libs/skia/tools/timer/
    PUBLIC ../../libs/algorithm/include/
    PUBLIC ../../libs/boost/core/include/
    PUBLIC ../../libs/boost/config/include/
    PUBLIC ../../libs/boost/range/include/
    PUBLIC ../../libs/boost/preprocessor/include/
    PUBLIC ../../libs/boost/mpl/include/
    PUBLIC ../../libs/boost/type_traits/include/
    PUBLIC ../../libs/boost/iterator/include/
    PUBLIC ../../libs/boost/assert/include/
    PUBLIC ../../libs/boost/static_assert/include/
    PUBLIC ../../libs/boost/detail/include/
    PUBLIC ../../libs/boost/concept_check/include/
    PUBLIC ../../libs/boost/utility/include/
    PUBLIC /usr/local/include/
    )

target_link_libraries(pesto
    "jpeg"
    "tinyxml2"
    "png"
    "sdl2"
)

## wasm
#target_link_libraries(pesto
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/skia/out/Build-wasm-Release/Release/libskia.a
#    )

# regular
target_link_libraries(pesto
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/skia/out/Clang/libskia.a
)


target_link_libraries(pesto
    "-framework CoreFoundation"
    "-framework CoreGraphics"
    "-framework CoreText"
    "-framework OpenGL"
#    "-framework CoreServices"
)
