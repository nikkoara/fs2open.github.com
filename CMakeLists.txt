CMAKE_MINIMUM_REQUIRED(VERSION 3.4)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Make sure the source and binary directory are not the sample
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(FATAL_ERROR "Binary and source directory can not be the same! Choose another build directory.")
endif()

include(cmake/util.cmake)

# Set a few policies
set_policy(CMP0051 OLD)
set_policy(CMP0053 NEW)
set_policy(CMP0054 NEW)

SET(FSO_PROJECT_DESCRIPTION "FreeSpace2 open-source project")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Executable output path")
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Library output path")

# CMAKE_INSTALL_PREFIX is defined but also overwritten by the PROJECT command so we need to set it later
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(RESET_INSTALL_PREFIX TRUE)
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)

# Reset this variable
SET(TARGET_COPY_FILES "" CACHE INTERNAL "" FORCE)
SET(TARGET_COPY_DIRS "" CACHE INTERNAL "" FORCE)

### Define options and editable variables in the section below:
# This is only needed for single configuration generators so don't annoy others
if(NOT CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build configuration used." FORCE)
  message(WARNING "CMAKE_BUILD_TYPE was not specified, defaulting to Release configuration.")
endif(NOT CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)

# Override standard CMake configurations
set(CMAKE_CONFIGURATION_TYPES Debug Release FastDebug CACHE STRING "Available configurations" FORCE)

PROJECT(FS2_Open)

find_program(CCACHE_PROGRAM ccache)
# Currently only GCC support ccache properly without generating additional warnings
if(CCACHE_PROGRAM AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  message(STATUS "Using ccache for compilation: ${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

IF(CMAKE_CROSSCOMPILING)
  SET(IMPORT_EXE_DIR "IMPORT_EXE_DIR-NOTFOUND" CACHE FILEPATH "Required for a cross compiling build, point to binary directory of a cmake build for the real OS")
  INCLUDE(${IMPORT_EXE_DIR}/ImportExecutables.cmake)
ENDIF(CMAKE_CROSSCOMPILING)

# Check if the external modules exists
IF(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/rpavlik-cmake-modules/launcher-templates")
  message(FATAL_ERROR "External submodules could not be found. Please make sure you have updated your submodules.")
endif()

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/finder")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/rpavlik-cmake-modules")

IF(RESET_INSTALL_PREFIX)
  IF(NOT $ENV{FS2PATH} STREQUAL "")
    IF(NOT IS_DIRECTORY $ENV{FS2PATH})
      MESSAGE(WARNING "The path '$ENV{FS2PATH}' of the environment variable FS2PATH does not point to a directory! Install prefix wasn't set.")
    ELSE(NOT IS_DIRECTORY $ENV{FS2PATH})
      SET(CMAKE_INSTALL_PREFIX "$ENV{FS2PATH}" CACHE FILEPATH "Install path" FORCE)
      FILE(TO_CMAKE_PATH ${CMAKE_INSTALL_PREFIX} CMAKE_INSTALL_PREFIX)
    ENDIF(NOT IS_DIRECTORY $ENV{FS2PATH})
  ELSE(NOT $ENV{FS2PATH} STREQUAL "")
    MESSAGE(STATUS "NO FS2PATH environment variable found, you can define this to point to your freespace install but it is not required.")
  ENDIF(NOT $ENV{FS2PATH} STREQUAL "")
ENDIF(RESET_INSTALL_PREFIX)

IF(UNIX)
  option(FSO_BUILD_APPIMAGE "Build an AppImage package" OFF)
ENDIF(UNIX)

OPTION(FSO_DEVELOPMENT_MODE "Generate binaries in development mode, only use if you know what you're doing!" OFF)

IF(WIN32 OR APPLE)
  # On windows and mac the default should be to always build the included libraries
  SET(FSO_BUILD_INCLUDED_LIBS_DEFAULT ON)
ELSE()
  SET(FSO_BUILD_INCLUDED_LIBS_DEFAULT OFF)
ENDIF()

OPTION(FSO_BUILD_INCLUDED_LIBS "Build and use the included libraries istead of using the system headers and libraries" ${FSO_BUILD_INCLUDED_LIBS_DEFAULT})
OPTION(FSO_FATAL_WARNINGS "Determines if warnings in the build are considered fatal errors, primarily used for CI" OFF)
SET(FSO_FREESPACE_PATH "${CMAKE_INSTALL_PREFIX}" CACHE FILEPATH "The path of the FreeSpace directory you want to use. Please note that you will have to change CMAKE_INSTALL_PREFIX if you change this at some point.")

SET(FSO_RUN_ARGUMENTS "" CACHE STRING "Additional arguments passed to a generated executable when run with the generated build files.")

option(FSO_INSTALL_DEBUG_FILES "Install some debug files (currently only PDB files on windows)" OFF)

MARK_AS_ADVANCED(FORCE FSO_CMAKE_DEBUG)
MARK_AS_ADVANCED(FORCE FSO_BUILD_INCLUDED_LIBS)
MARK_AS_ADVANCED(FORCE FSO_DEVELOPMENT_MODE)
MARK_AS_ADVANCED(FORCE FSO_FATAL_WARNINGS)
mark_as_advanced(FORCE FSO_INSTALL_DEBUG_FILES)

INCLUDE(globals)
INCLUDE(toolchain)
INCLUDE(platforms)
include(version)

INCLUDE(headers)

SET(GENERATED_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated_source)

include(generateHeaders)

if(FSO_BUILD_APPIMAGE)
  include(appimage)
endif()

include(clang-tidy)

#
# This includes source code for some tools, either used in the build or also for
# something else
#
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(code)
ADD_SUBDIRECTORY(freespace2)

if ("${CMAKE_VERSION}" VERSION_GREATER "3.5")
  # Default to using Freespace2 as startup project if CMake supports it.
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Freespace2)
endif()

IF(MSVC_USE_RUNTIME_DLL)
  SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BINARY_DESTINATION}")
  INCLUDE(InstallRequiredSystemLibraries)
ENDIF(MSVC_USE_RUNTIME_DLL)

include(package)

include(doxygen)

# Print used options to log
IF (WIN32)
  message(STATUS "Using text to speech: ${FSO_USE_SPEECH}")
  message(STATUS "Using voice recogition: ${FSO_USE_VOICEREC}")
elseif(UNIX)
  message(STATUS "Building AppImage: ${FSO_BUILD_APPIMAGE}")
ENDIF()
message(STATUS "Building FSO tools: ${FSO_BUILD_TOOLS}")
message(STATUS "Fatal warnings: ${FSO_FATAL_WARNINGS}")
