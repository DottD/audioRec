# AudioRec global CMakeLists
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(AudioRec)

#---Group headers and sources into variable
file(GLOB_RECURSE HEADERS Headers/*.hpp)
file(GLOB_RECURSE SOURCES Sources/*.cpp)
file(GLOB_RECURSE RESOURCES Resources/*.qrc)

#---Find the necessary packages
find_package(Qt5 COMPONENTS Core Gui Widgets Multimedia REQUIRED)
find_package(Armadillo REQUIRED)
find_package(SFML COMPONENTS audio)

#---Add the subprojects to this workspace
add_subdirectory(GUI)
add_subdirectory(matLib)

#---Add C++14 support to the project
set_property(TARGET AudioRec PROPERTY CXX_STANDARD 14)