# AudioRec/matLib application CMakeLists
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(matLib)

#---Group headers and sources into variables
file(GLOB_RECURSE HEADERS Headers/*.hpp)
file(GLOB_RECURSE SOURCES Sources/*.cpp)

#---Create the static library
add_library(matLib ${SOURCES} ${HEADERS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${ARMADILLO_INCLUDE_DIRS} ${ALGLIB_INCLUDES})
target_link_libraries(matLib ${ARMADILLO_LIBRARIES} ${ALGLIB_LIBRARIES} Qt5::Core)

#---Add C++14 support to the project
set_property(TARGET matLib PROPERTY CXX_STANDARD 14)