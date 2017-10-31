#
# <root>/CMakeLists.txt
#

cmake_minimum_required(VERSION 2.8)
#### P r o j e c t   D e f i n i t i o n  ##################################
project(preprocessor LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH}
)


#### Cruft  Library ####################################################
if(NOT DEFINED CRUFT_INCLUDE_DIR)
    set(CRUFT_INCLUDE_DIR)
endif()
set(CRUFT_INCLUDE_DIR ${${CRUFT_INCLUDE_DIR}} CACHE PATH "Cruft includes directory path")

if(NOT DEFINED CRUFT_LIBRARY_DIR)
    set(CRUFT_LIBRARY_DIR)
endif()
set(CRUFT_LIBRARY_DIR ${${CRUFT_LIBRARY_DIR}} CACHE PATH "Cruft library file (.a,.dylib,.lib).")


#### Intel TBB Library ##################################################
if(NOT DEFINED TBB_INCLUDE_DIR)
    set(TBB_INCLUDE_DIR)
endif()
set(TBB_INCLUDE_DIR ${${TBB_INCLUDE_DIR}} CACHE PATH "Cruft includes directory path")

if(NOT DEFINED TBB_LIBRARY_DIR)
    set(TBB_LIBRARY_DIR)
endif()
set(TBB_LIBRARY_DIR ${${TBB_LIBRARY_DIR}} CACHE PATH "Directory with intel tbb libs.")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") 
    add_definitmdions(-DTBB_USE_DEBUG=1)
endif()

#### Definitions ##########################################################
# Windows requires a few extra definitions.
if (WIN32)
    add_definitions(-DNOMINMAX)     #Disable the overrides of std::min/max in Windows.h
endif (WIN32)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall \
    -Wno-missing-braces \
   -fdiagnostics-color=auto \
    -pthread" )
endif (UNIX)


#### T h i r d   P a r t y  ################################################

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

find_package(GLM REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

find_library(CRUFT_DEBUG_LIB cruftd HINTS "${CRUFT_LIBRARY_DIR}")
find_library(CRUFT_RELEASE_LIB cruft HINTS "${CRUFT_LIBRARY_DIR}")
find_library(TBB_DEBUG_LIB tbb_debug HINTS "${TBB_LIBRARY_DIR}")
find_library(TBB_RELEASE_LIB tbb HINTS "${TBB_LIBRARY_DIR}")

include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/tclap/include"
        "${GLM_INCLUDE_DIRS}"
        "${CRUFT_INCLUDE_DIR}"
        "${TBB_INCLUDE_DIR}"
        "${Boost_INCLUDE_DIR}"
)


add_subdirectory(preproc)
add_subdirectory(gradvol)
#add_subdirectory(rawhist)
#add_subdirectory(resample)
#add_subdirectory(gmag)
add_subdirectory(concat)
