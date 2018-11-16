cmake_minimum_required(VERSION 3.10)
project(pesto)

#add_library(catch ../../../libs/Catch2/single_include/catch.hpp)
#set_target_properties(catch PROPERTIES LINKER_LANGUAGE CXX)

include_directories(../../../libs/Catch2/single_include/catch2)

include_directories(
    ../../../libs/skia/include/core
    ../../../libs/skia/include/config
    ../../../libs/skia/include/gpu
    ../../../libs/skia/include/utils
    ../../../libs/skia/third_party/externals/sdl/include/
    ../../../libs/skia/src/gpu
    ../../../libs/skia/src/core
    ../../../libs/skia/src/image/
    ../../../libs/skia/tools/
    ../../../libs/skia/include/views/
    ../../../libs/skia/include/effects/
    ../../../libs/skia/include/private/
    ../../../libs/skia/tools/timer/
    ../../../libs/algorithm/include/
)
link_libraries(pesto)

file(GLOB TESTS test*.cpp)
foreach(file ${TESTS})
    get_filename_component(name ${file} NAME_WE)
    MESSAGE("Test found pesto_${name}.")
    add_executable(pesto_${name} ${file})
endforeach()

