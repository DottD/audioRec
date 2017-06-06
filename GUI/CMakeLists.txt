# AudioRec/GUI application CMakeLists
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(GUI)

#---Group headers and sources into variables
file(GLOB_RECURSE HEADERS Headers/*.hpp)
file(GLOB_RECURSE SOURCES Sources/*.cpp)
file(GLOB_RECURSE RESOURCES Resources/*.qrc)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
file(GLOB_RECURSE UI_FILES Resources/*.ui)
qt5_wrap_ui(UI_H ${UI_FILES})
# CMAKE_AUTOMOC in ON so the MOC headers will be automatically wrapped.
set(CMAKE_AUTORCC ON)

#---Create the executable
if(APPLE)
add_executable(AudioRec MACOSX_BUNDLE ${SOURCES} ${UI_H} ${HEADERS} ${RESOURCES})
elseif(UNIX AND NOT APPLE)
add_executable(AudioRec ${SOURCES} ${UI_H} ${HEADERS} ${RESOURCES})
endif()
target_link_libraries(AudioRec ${ARMADILLO_LIBRARIES} ${SFML_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Charts Qt5::Multimedia matLib)

#---Include headers directories to the executable
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
	${ARMADILLO_INCLUDE_DIRS}
	../matLib)

#---Add C++14 support to the project
set_property(TARGET AudioRec PROPERTY CXX_STANDARD 14)