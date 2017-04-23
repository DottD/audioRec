# AudioRec/GUI application CMakeLists
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(GUI)

#---Group headers and sources into variable
file(GLOB_RECURSE HEADERS Headers/*.hpp)
file(GLOB_RECURSE SOURCES Sources/*.cpp)
file(GLOB_RECURSE RESOURCES Resources/*.qrc)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
file(GLOB_RECURSE UI_FILES Resources/*.ui)
qt5_wrap_ui(UI_H ${UI_FILES})
# CMAKE_AUTOMOC in ON so the MOC headers will be automatically wrapped.

#---Create a shared library with geneated dictionary
if(APPLE)
add_executable(AudioRec MACOSX_BUNDLE ${SOURCES} ${UI_H} ${HEADERS} ${RESOURCES})
elseif(UNIX AND NOT APPLE)
add_executable(AudioRec ${SOURCES} ${UI_H} ${HEADERS} ${RESOURCES})
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(AudioRec ${ROOT_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets)

#---Add C++14 support to the project
set_property(TARGET AudioRec PROPERTY CXX_STANDARD 14)