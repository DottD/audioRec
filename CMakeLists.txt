# Completely Automatic Voice Analysis
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(CAVA)

# Set the version number
set(CAVA_MAJOR_VERSION 1)
set(CAVA_MINOR_VERSION 0)
set(CAVA_PATCH_VERSION 0)
set(CAVA_VERSION
  ${CAVA_MAJOR_VERSION}.${CAVA_MINOR_VERSION}.${CAVA_PATCH_VERSION})

# Add the BUILD_SHARED_LIBS option, that automatically deal with add_library
option(BUILD_SHARED_LIBS "Whether you want a static or shared build" ON)

# Add a default build type (not sure if working with multiconfig generators)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif(NOT CMAKE_BUILD_TYPE)

# Qt library debug mode (depending on )
if(CMAKE_BUILD_TYPE EQUAL "Debug")
  set(DYLD_IMAGE_SUFFIX _debug CACHE STRING "Whether using debug mode or not" FORCE)
else()
  set(DYLD_IMAGE_SUFFIX "" CACHE STRING "Whether using debug mode or not" FORCE)
endif()

# Find the necessary packages
if(NOT BUILD_SHARED_LIBS)
  set(SFML_STATIC_LIBRARIES TRUE)
endif()
set(ALGLIB_FIND_MODULE_PATH "" CACHE PATH "Path to the FindALGLIB.cmake module")
set(SFML_FIND_MODULE_PATH "" CACHE PATH "Path to the FindSFML.cmake module")
set(CMAKE_MODULE_PATH "${ALGLIB_FIND_MODULE_PATH};${SFML_FIND_MODULE_PATH}")
find_package(Qt5 COMPONENTS Core Gui Widgets Charts Multimedia REQUIRED)
find_package(Armadillo REQUIRED)
find_package(SFML COMPONENTS audio system REQUIRED)
if(SFML_FOUND)
set(SFML_LIBRARIES sfml-audio)
endif()
find_package(ALGLIB REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Spectrum)
find_package(QAlgorithm REQUIRED)

message(STATUS "Root use file: ${ROOT_USE_FILE}")
include(${ROOT_USE_FILE})
string(REGEX REPLACE "-std=c\\+\\+[0-9]+" "" ROOT_CXX_FLAGS ${ROOT_CXX_FLAGS})
add_definitions(${ROOT_CXX_FLAGS})

# Include the right directories to the search path
include_directories("${PROJECT_SOURCE_DIR}/Headers" ${PROJECT_BINARY_DIR} ${ARMADILLO_INCLUDE_DIRS} ${ALGLIB_INCLUDES} ${QAlgorithm_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create the mathematical library
set(CMAKE_AUTOMOC ON)
file(GLOB_RECURSE UMF_HEADERS Headers/UMF/*.hpp)
file(GLOB_RECURSE UMF_SOURCES Sources/UMF/*.cpp)
if(BUILD_SHARED_LIBS)
add_library(UMF ${UMF_SOURCES} ${UMF_HEADERS}) # Useful Mathematical Functions
else()
add_library(UMF STATIC ${UMF_SOURCES} ${UMF_HEADERS}) # Useful Mathematical Functions
endif()
target_link_libraries(UMF ${ARMADILLO_LIBRARIES} ${ALGLIB_LIBRARIES} ${QAlgorithm_LIBRARIES} ${ROOT_LIBRARIES} Qt5::Core)

# Create the Audio Analysis library
file(GLOB_RECURSE AA_HEADERS Headers/AA/*.hpp)
file(GLOB_RECURSE AA_SOURCES Sources/AA/*.cpp)
if(BUILD_SHARED_LIBS)
add_library(AA ${AA_SOURCES} ${AA_HEADERS}) # Audio Analysis
else()
add_library(AA ${AA_SOURCES} ${AA_HEADERS}) # Audio Analysis
endif()
target_link_libraries(AA ${ARMADILLO_LIBRARIES} ${ALGLIB_LIBRARIES} ${SFML_LIBRARIES} ${QAlgorithm_LIBRARIES} ${ROOT_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia UMF)

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
target_link_libraries(CAVA ${ARMADILLO_LIBRARIES} ${SFML_LIBRARIES} ${QAlgorithm_LIBRARIES} ${ROOT_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Charts Qt5::Multimedia UMF AA)
