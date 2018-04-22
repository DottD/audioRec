# QAlgorithm CMakeLists
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
cmake_policy(SET CMP0020 NEW)
project(QAlgorithm)

# Set the version number
set(QAlgorithm_MAJOR_VERSION 1)
set(QAlgorithm_MINOR_VERSION 0)
set(QAlgorithm_PATCH_VERSION 0)
set(QAlgorithm_VERSION
  ${QAlgorithm_MAJOR_VERSION}.${QAlgorithm_MINOR_VERSION}.${QAlgorithm_PATCH_VERSION})

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
find_package(Qt5 COMPONENTS Core REQUIRED)

# Group headers and sources into variable
file(GLOB_RECURSE HEADERS Headers/*.hpp)
file(GLOB_RECURSE SOURCES Sources/*.cpp)

include_directories(${PROJECT_SOURCE_DIR})

# Instruct CMake to automatically run the moc
set(CMAKE_AUTOMOC ON)

# Create a library from the sources
add_library(QAlgorithm ${SOURCES} ${HEADERS})

# Add libraries to the target
target_link_libraries(QAlgorithm Qt5::Core)

# Add C++14 support to the project
set_property(TARGET QAlgorithm PROPERTY CXX_STANDARD 14)

# # Check if CMAKE_INSTALL_PREFIX is already defined
# message(WARNING "Remember to choose an installation directory, do it editing CMAKE_INSTALL_PREFIX")

# # Install the library to the system
# install(TARGETS QAlgorithm
#   # IMPORTANT: Add the QAlgorithm library to the "export-set"
#   EXPORT QAlgorithmTarget
#   RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" COMPONENT bin
#   LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" COMPONENT shlib
#   ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" COMPONENT shlib
#   PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
#     COMPONENT dev)
# install(FILES ${HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include")

# # Add all targets to the build-tree export set
# export(TARGETS QAlgorithm
#   FILE "${PROJECT_BINARY_DIR}/QAlgorithmTarget.cmake")
 
# # Export the package for use from the build-tree
# # (this registers the build-tree with a global CMake-registry)
# export(PACKAGE QAlgorithm)
 
# # Create the QAlgorithmConfig.cmake and QAlgorithmConfigVersion files
# # ... for the build tree
# #set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
# set(CONF_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
# configure_file(QAlgorithmConfig.cmake.in
#   "${PROJECT_BINARY_DIR}/QAlgorithmConfig.cmake" @ONLY)
# # ... for the install tree
# set(CONF_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
# configure_file(QAlgorithmConfig.cmake.in
#   "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/QAlgorithmConfig.cmake" @ONLY)
# # ... for both
# configure_file(QAlgorithmConfigVersion.cmake.in
#   "${PROJECT_BINARY_DIR}/QAlgorithmConfigVersion.cmake" @ONLY)

# # Install the QAlgorithmConfig.cmake and QAlgorithmConfigVersion.cmake
# install(FILES
#   "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/QAlgorithmConfig.cmake"
#   "${PROJECT_BINARY_DIR}/QAlgorithmConfigVersion.cmake"
#   DESTINATION "${CMAKE_INSTALL_PREFIX}" COMPONENT dev)
 
# # Install the export set for use with the install-tree
# install(EXPORT QAlgorithmTarget DESTINATION
#   "${CMAKE_INSTALL_PREFIX}" COMPONENT dev)
