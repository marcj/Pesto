cmake_minimum_required(VERSION 3.10)
project(deepkit)

add_definitions(-DTRACY_ENABLE)

include_directories(libs/tracy/)

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3")

#include_directories(libs/RxCpp/Rx/v2/src)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# osx only c++1z,
# otherwise its called c++17
ADD_DEFINITIONS(
        -std=c++1z
)

add_subdirectory(../Pesto/src/pesto/)
add_subdirectory(src/server/)
add_subdirectory(src/cli/)

