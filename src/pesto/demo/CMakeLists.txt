cmake_minimum_required(VERSION 3.10)
project(pesto)

find_package(OpenGL REQUIRED)

file(GLOB EXAMPLES example*.cpp)
foreach(file ${EXAMPLES})
    get_filename_component(name ${file} NAME_WE)
    MESSAGE("Demo example found ${name}.")
    add_executable(pesto_demo_${name} ${file})
    target_link_libraries(
            pesto_demo_${name}
            pesto
    )
endforeach()