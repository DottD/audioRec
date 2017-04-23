# AudioRec global CMakeLists
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(AudioRec)

#---Group headers and sources into variable
file(GLOB_RECURSE HEADERS Headers/*.hpp)
file(GLOB_RECURSE SOURCES Sources/*.cpp)
file(GLOB_RECURSE RESOURCES Resources/*.qrc)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED)
#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
# Include the Qt directories in the project
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

#---Add the subprojects to this workspace
add_subdirectory(GUI)

#---Add C++14 support to the project
set_property(TARGET AudioRec PROPERTY CXX_STANDARD 14)