# Completely Automatic Voice Analysis
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(CAVA)

# Set the version number
set(CAVA_MAJOR_VERSION 1)
set(CAVA_MINOR_VERSION 0)
set(CAVA_PATCH_VERSION 0)
set(CAVA_VERSION
  ${CAVA_MAJOR_VERSION}.${CAVA_MINOR_VERSION}.${CAVA_PATCH_VERSION})

add_subdirectory(QAlgorithm)

# Add the BUILD_SHARED_LIBS option, that automatically deal with add_library
option(BUILD_SHARED_LIBS "Whether you want a static or shared build" ON)

# Qt library debug mode on (comment out this line for release mode)
option(WITH_QT_DEBUG "Whether to link to Qt with debugging symbols" OFF)
if(WITH_QT_DEBUG)
  message(STATUS "Adding Qt debugging symbols")
  set(DYLD_IMAGE_SUFFIX _debug CACHE STRING "Whether using debug mode or not")
else()
  message(STATUS "Without Qt debugging symbols")
  unset(DYLD_IMAGE_SUFFIX CACHE)
endif()

# Find the necessary packages
set(ALGLIB_FIND_MODULE_PATH "." CACHE PATH "Path to the FindALGLIB.cmake module")
set(SFML_FIND_MODULE_PATH "." CACHE PATH "Path to the FindSFML.cmake module")
set(CMAKE_MODULE_PATH "${ALGLIB_FIND_MODULE_PATH};${SFML_FIND_MODULE_PATH}")
find_package(Qt5 COMPONENTS Core Gui Widgets Charts Multimedia REQUIRED)
find_package(Armadillo REQUIRED)
find_package(SFML COMPONENTS audio system)
find_package(ALGLIB REQUIRED)
# find_package(QAlgorithm REQUIRED)

# Qt library debug mode on (comment out this line for release mode)
set(DYLD_IMAGE_SUFFIX _debug CACHE STRING "Whether using debug mode or not")

# Include the right directories to the search path
include_directories("${PROJECT_SOURCE_DIR}/Headers" ${PROJECT_BINARY_DIR} ${ARMADILLO_INCLUDE_DIRS} ${ALGLIB_INCLUDES} QAlgorithm/Headers)
# include_directories("${PROJECT_SOURCE_DIR}/Headers" ${PROJECT_BINARY_DIR} ${ARMADILLO_INCLUDE_DIRS} ${ALGLIB_INCLUDES} ${QAlgorithm_INCLUDE_DIRS})

# Create the mathematical library
set(CMAKE_AUTOMOC ON)
file(GLOB_RECURSE UMF_HEADERS Headers/UMF/*.hpp)
file(GLOB_RECURSE UMF_SOURCES Sources/UMF/*.cpp)
add_library(UMF ${UMF_SOURCES} ${UMF_HEADERS}) # Useful Mathematical Functions
target_link_libraries(UMF ${ARMADILLO_LIBRARIES} ${ALGLIB_LIBRARIES} QAlgorithm Qt5::Core)
# target_link_libraries(UMF ${ARMADILLO_LIBRARIES} ${ALGLIB_LIBRARIES} ${QAlgorithm_LIBRARIES} Qt5::Core)
set_property(TARGET UMF PROPERTY CXX_STANDARD 14)

# Create the Audio Analysis library
file(GLOB_RECURSE AA_HEADERS Headers/AA/*.hpp)
file(GLOB_RECURSE AA_SOURCES Sources/AA/*.cpp)
add_library(AA ${AA_SOURCES} ${AA_HEADERS})
target_link_libraries(AA ${ARMADILLO_LIBRARIES} ${ALGLIB_LIBRARIES} ${SFML_LIBRARIES} QAlgorithm Qt5::Core Qt5::Gui UMF)
# target_link_libraries(AA ${ARMADILLO_LIBRARIES} ${ALGLIB_LIBRARIES} ${SFML_LIBRARIES} ${QAlgorithm_LIBRARIES} Qt5::Core Qt5::Gui UMF)
set_property(TARGET AA PROPERTY CXX_STANDARD 14)

# Create the executable with GUI
file(GLOB_RECURSE UI_FILES Resources/GUI/*.ui)
qt5_wrap_ui(UI_H ${UI_FILES})
# CMAKE_AUTOMOC in ON so the MOC headers will be automatically wrapped.
set(CMAKE_AUTORCC ON)
file(GLOB_RECURSE GUI_HEADERS Headers/GUI/*.hpp)
file(GLOB_RECURSE GUI_SOURCES Sources/GUI/*.cpp)
file(GLOB_RECURSE GUI_RESOURCES Resources/GUI/*.qrc)
if(APPLE)
add_executable(CAVA MACOSX_BUNDLE ${GUI_SOURCES} ${UI_H} ${GUI_HEADERS} ${GUI_RESOURCES})
elseif(UNIX AND NOT APPLE)
add_executable(CAVA ${GUI_SOURCES} ${UI_H} ${GUI_HEADERS} ${GUI_RESOURCES})
endif()
target_link_libraries(CAVA ${ARMADILLO_LIBRARIES} ${SFML_LIBRARIES} QAlgorithm Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Charts Qt5::Multimedia UMF AA)
# target_link_libraries(CAVA ${ARMADILLO_LIBRARIES} ${SFML_LIBRARIES} ${QAlgorithm_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Charts Qt5::Multimedia UMF AA)
